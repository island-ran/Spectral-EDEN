#include <mr_dtg_plus/mr_dtg_plus.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <unordered_map>

using namespace DTGPlus;

namespace {
constexpr double kUnreachableDistance = 999998.0;
constexpr double kBlockedEdgeDistance = 200000.0;

double Jaccard(const std::unordered_set<uint32_t> &lhs,
               const std::unordered_set<uint32_t> &rhs){
    if(lhs.empty() && rhs.empty()) return 1.0;
    size_t intersection = 0;
    for(const uint32_t id : lhs){
        if(rhs.count(id) != 0U) ++intersection;
    }
    const size_t union_size = lhs.size() + rhs.size() - intersection;
    return union_size == 0U ? 0.0
                            : static_cast<double>(intersection) / static_cast<double>(union_size);
}

// Farthest-point sampling: select k landmarks from a set of points with
// supplied positions.  Returns indices into the positions vector.
std::vector<size_t> FarthestPointSampling(
    const std::vector<Eigen::Vector3d> &positions, size_t k){
    const size_t n = positions.size();
    if(n == 0 || k == 0) return {};
    if(k >= n){
        std::vector<size_t> all(n);
        for(size_t i = 0; i < n; ++i) all[i] = i;
        return all;
    }
    std::vector<size_t> landmarks;
    landmarks.reserve(k);
    std::vector<double> min_dist(n, std::numeric_limits<double>::infinity());

    // First landmark: centroid-closest point
    Eigen::Vector3d centroid = Eigen::Vector3d::Zero();
    for(const auto &p : positions) centroid += p;
    centroid /= static_cast<double>(n);
    size_t first = 0;
    double first_dist = (positions[0] - centroid).squaredNorm();
    for(size_t i = 1; i < n; ++i){
        const double d = (positions[i] - centroid).squaredNorm();
        if(d < first_dist){ first_dist = d; first = i; }
    }
    landmarks.push_back(first);
    for(size_t i = 0; i < n; ++i){
        min_dist[i] = (positions[i] - positions[first]).squaredNorm();
    }

    while(landmarks.size() < k){
        size_t farthest = 0;
        double farthest_dist = min_dist[0];
        for(size_t i = 1; i < n; ++i){
            if(min_dist[i] > farthest_dist){
                farthest_dist = min_dist[i];
                farthest = i;
            }
        }
        landmarks.push_back(farthest);
        for(size_t i = 0; i < n; ++i){
            const double d = (positions[i] - positions[farthest]).squaredNorm();
            if(d < min_dist[i]) min_dist[i] = d;
        }
    }
    return landmarks;
}
}

// ── Path clearance estimation (unchanged) ──

double MultiDtgPlus::EstimatePathClearance(
    const list<Eigen::Vector3d> &path) const{
    if(path.empty() || BM_ == nullptr || LRM_ == nullptr) return 0.0;

    const double maximum = spectral_config_.clearance_reference;
    const Eigen::Vector3d robot_size = LRM_->GetRobotSize();
    const size_t max_samples = static_cast<size_t>(
        std::max(2, spectral_v4_config_.clearance_max_samples));
    const size_t stride = std::max<size_t>(1,
        (path.size() + max_samples - 1) / max_samples);
    double path_clearance = maximum;
    size_t index = 0;
    for(auto point = path.begin(); point != path.end(); ++point, ++index){
        const bool is_endpoint = point == path.begin() || std::next(point) == path.end();
        if(!is_endpoint && index % stride != 0U) continue;
        if(BM_->PosBBXOccupied(*point, robot_size)) return 0.0;

        double low = 0.0;
        double high = maximum;
        for(int step = 0; step < spectral_v4_config_.clearance_binary_steps; ++step){
            const double radius = 0.5 * (low + high);
            const Eigen::Vector3d expanded = robot_size +
                Eigen::Vector3d::Ones() * (2.0 * radius);
            if(BM_->PosBBXOccupied(*point, expanded)) high = radius;
            else low = radius;
        }
        path_clearance = std::min(path_clearance, low);
    }
    return std::max(0.0, path_clearance);
}

double MultiDtgPlus::EdgeClearance(const hhe_ptr &edge){
    if(edge == nullptr) return 0.0;
    if(!std::isfinite(edge->clearance_)){
        edge->clearance_ = EstimatePathClearance(edge->path_);
    }
    return std::max(0.0, edge->clearance_);
}

double MultiDtgPlus::CachedPathClearance(const h_ptr &start,
    const HPathCacheEntry &path_entry){
    if(path_entry.h_path.empty()) return 0.0;
    auto node = path_entry.h_path.begin();
    h_ptr previous = *node;
    if(previous == nullptr) previous = start;
    if(previous == start) ++node;

    double clearance = spectral_config_.clearance_reference;
    for(; node != path_entry.h_path.end(); ++node){
        if(*node == nullptr || previous == nullptr) return 0.0;
        if(*node == previous) continue;
        hhe_ptr edge;
        h_ptr current = *node;
        if(!GetEdge(previous, current, edge)) return 0.0;
        clearance = std::min(clearance, EdgeClearance(edge));
        previous = *node;
    }
    return std::max(0.0, clearance);
}

// ── Edge betweenness (unchanged) ──

bool MultiDtgPlus::ComputeEdgeBetweenness(
    SpectralGraphSnapshot &snapshot, std::string &reason) const{
    struct AdjacentEdge{
        size_t to;
        size_t edge_index;
        double length;
    };
    const size_t node_count = snapshot.nodes.size();
    std::unordered_map<uint32_t, size_t> index_by_id;
    for(size_t i = 0; i < node_count; ++i) index_by_id[snapshot.nodes[i].h_id] = i;
    vector<vector<AdjacentEdge>> adjacency(node_count);
    for(size_t i = 0; i < snapshot.edges.size(); ++i){
        const auto from = index_by_id.find(snapshot.edges[i].from_h_id);
        const auto to = index_by_id.find(snapshot.edges[i].to_h_id);
        if(from == index_by_id.end() || to == index_by_id.end() ||
           !std::isfinite(snapshot.edges[i].length) || snapshot.edges[i].length <= 0.0){
            reason = "cannot compute edge betweenness on an invalid support edge";
            return false;
        }
        adjacency[from->second].push_back({to->second, i, snapshot.edges[i].length});
        adjacency[to->second].push_back({from->second, i, snapshot.edges[i].length});
    }

    vector<double> edge_centrality(snapshot.edges.size(), 0.0);
    const double tolerance = 1.0e-9;
    using QueueEntry = pair<double, size_t>;
    for(size_t source = 0; source < node_count; ++source){
        vector<double> distance(node_count, std::numeric_limits<double>::infinity());
        vector<double> path_count(node_count, 0.0);
        vector<double> dependency(node_count, 0.0);
        vector<vector<pair<size_t, size_t>>> predecessors(node_count);
        vector<size_t> order;
        std::priority_queue<QueueEntry, vector<QueueEntry>, std::greater<QueueEntry>> open;
        distance[source] = 0.0;
        path_count[source] = 1.0;
        open.push({0.0, source});

        while(!open.empty()){
            const QueueEntry current = open.top();
            open.pop();
            if(current.first > distance[current.second] + tolerance) continue;
            order.emplace_back(current.second);
            for(const AdjacentEdge &edge : adjacency[current.second]){
                const double candidate = current.first + edge.length;
                if(candidate + tolerance < distance[edge.to]){
                    distance[edge.to] = candidate;
                    path_count[edge.to] = path_count[current.second];
                    predecessors[edge.to].clear();
                    predecessors[edge.to].push_back({current.second, edge.edge_index});
                    open.push({candidate, edge.to});
                }
                else if(std::abs(candidate - distance[edge.to]) <= tolerance){
                    path_count[edge.to] += path_count[current.second];
                    predecessors[edge.to].push_back({current.second, edge.edge_index});
                }
            }
        }

        for(auto node = order.rbegin(); node != order.rend(); ++node){
            if(path_count[*node] <= spectral_config_.numeric_epsilon) continue;
            for(const auto &predecessor : predecessors[*node]){
                const double contribution = path_count[predecessor.first] /
                    path_count[*node] * (1.0 + dependency[*node]);
                edge_centrality[predecessor.second] += contribution;
                dependency[predecessor.first] += contribution;
            }
        }
    }
    for(size_t i = 0; i < snapshot.edges.size(); ++i){
        snapshot.edges[i].betweenness = 0.5 * edge_centrality[i];
    }
    return true;
}

// ── Sparse support spectral graph (kNN + MST backbone + corridor compression) ──

bool MultiDtgPlus::BuildSparseSupportSpectralGraph(
    const GlobalRouteContext &context, SpectralGraphSnapshot &snapshot,
    std::string &reason){
    SpectralGraphSnapshot raw_snapshot;
    raw_snapshot.graph_version = dtg_version_;
    std::unordered_map<uint32_t, h_ptr> support_nodes;
    std::unordered_set<uint32_t> active_ids;
    std::unordered_set<uint32_t> preserved_ids;
    for(const auto &h : context.active_hnodes){
        if(h == nullptr) continue;
        support_nodes[h->id_] = h;
        active_ids.insert(h->id_);
        preserved_ids.insert(h->id_);
    }

    std::unordered_set<uint64_t> support_pairs;
    auto pair_distance = [&](size_t i, size_t j){
        // Use Euclidean distance between active H-node positions as a fast proxy
        // when no precomputed distance matrix entry is available.
        return (context.active_hnodes[i]->pos_ -
                context.active_hnodes[j]->pos_).norm();
    };
    for(size_t i = 0; i < context.active_hnodes.size(); ++i){
        vector<pair<double, size_t>> neighbours;
        neighbours.reserve(context.active_hnodes.size());
        for(size_t j = 0; j < context.active_hnodes.size(); ++j){
            if(i == j) continue;
            neighbours.push_back({pair_distance(i, j), j});
        }
        std::sort(neighbours.begin(), neighbours.end(),
            [&](const pair<double, size_t> &lhs, const pair<double, size_t> &rhs){
                if(lhs.first != rhs.first) return lhs.first < rhs.first;
                return context.active_hnodes[lhs.second]->id_ <
                       context.active_hnodes[rhs.second]->id_;
            });
        const size_t keep = std::min(neighbours.size(), static_cast<size_t>(
            std::max(1, spectral_v4_config_.spectral_knn)));
        for(size_t k = 0; k < keep; ++k){
            const size_t j = neighbours[k].second;
            support_pairs.insert(HPairKey(context.active_hnodes[i]->id_,
                                          context.active_hnodes[j]->id_));
        }
    }

    // MST backbone ensures connectivity
    const size_t active_count = context.active_hnodes.size();
    if(active_count > 1U){
        vector<double> key(active_count, std::numeric_limits<double>::infinity());
        vector<size_t> parent(active_count, active_count);
        vector<unsigned char> used(active_count, 0U);
        key[0] = 0.0;
        for(size_t iteration = 0; iteration < active_count; ++iteration){
            size_t current = active_count;
            for(size_t i = 0; i < active_count; ++i){
                if(used[i] == 0U &&
                   (current == active_count || key[i] < key[current])) current = i;
            }
            if(current == active_count || !std::isfinite(key[current])) break;
            used[current] = 1U;
            if(parent[current] != active_count){
                support_pairs.insert(HPairKey(
                    context.active_hnodes[current]->id_,
                    context.active_hnodes[parent[current]]->id_));
            }
            for(size_t candidate = 0; candidate < active_count; ++candidate){
                if(used[candidate] != 0U || candidate == current) continue;
                const double distance = pair_distance(current, candidate);
                if(std::isfinite(distance) && distance < key[candidate]){
                    key[candidate] = distance;
                    parent[candidate] = current;
                }
            }
        }
    }

    // Materialize paths for selected pairs
    std::unordered_map<uint32_t, vector<h_ptr>> missing_by_start;
    for(const uint64_t pair_key : support_pairs){
        const uint32_t from_id = static_cast<uint32_t>(pair_key >> 32U);
        const uint32_t to_id = static_cast<uint32_t>(pair_key & 0xffffffffU);
        const auto from = support_nodes.find(from_id);
        const auto to = support_nodes.find(to_id);
        if(from == support_nodes.end() || to == support_nodes.end()) continue;
        const auto cached = h_dist_map_.find(pair_key);
        if(cached == h_dist_map_.end() || cached->second.dtg_version != dtg_version_){
            missing_by_start[from_id].emplace_back(to->second);
        }
    }
    for(auto &missing : missing_by_start){
        h_ptr start = support_nodes.at(missing.first);
        MainTainDistMap(start, missing.second);
    }

    // Union of selected shortest paths
    for(const uint64_t key : support_pairs){
        const auto cached = h_dist_map_.find(key);
        if(cached == h_dist_map_.end() || cached->second.dtg_version != dtg_version_ ||
           cached->second.distance >= kBlockedEdgeDistance){
            continue;
        }
        for(const auto &h : cached->second.h_path){
            if(h != nullptr) support_nodes[h->id_] = h;
        }
    }

    // Preserve seeds and root-path nodes
    for(const auto &seed : context.seed_hnodes){
        if(seed != nullptr) preserved_ids.insert(seed->id_);
    }
    for(const auto &root_path : context.root_paths){
        for(const auto &h : root_path.second){
            if(h != nullptr) support_nodes[h->id_] = h;
        }
    }

    vector<uint32_t> ordered_ids;
    ordered_ids.reserve(support_nodes.size());
    for(const auto &entry : support_nodes){
        ordered_ids.emplace_back(entry.first);
        if(!entry.second->hf_edges_.empty()) preserved_ids.insert(entry.first);
    }
    std::sort(ordered_ids.begin(), ordered_ids.end());
    for(const uint32_t id : ordered_ids){
        raw_snapshot.nodes.emplace_back(id, active_ids.count(id) != 0U);
    }

    std::unordered_set<uint64_t> inserted_edges;
    for(const uint32_t id : ordered_ids){
        const h_ptr &h = support_nodes.at(id);
        for(const auto &edge : h->hh_edges_){
            if(edge == nullptr || edge->head_n_ == nullptr || edge->tail_n_ == nullptr) continue;
            if(!std::isfinite(edge->length_) || edge->length_ <= 0.0 ||
               edge->length_ >= kBlockedEdgeDistance) continue;
            const uint32_t head_id = edge->head_n_->id_;
            const uint32_t tail_id = edge->tail_n_->id_;
            if(support_nodes.count(head_id) == 0U || support_nodes.count(tail_id) == 0U) continue;
            const uint64_t key = HPairKey(head_id, tail_id);
            if(inserted_edges.insert(key).second){
                SpectralEdgeInput spectral_edge(head_id, tail_id, edge->length_);
                spectral_edge.clearance = EdgeClearance(edge);
                raw_snapshot.edges.emplace_back(spectral_edge);
            }
        }
    }

    if(raw_snapshot.edges.empty()){
        reason = "support graph contains no live H-H edges";
        return false;
    }

    last_raw_spectral_node_count_ = raw_snapshot.nodes.size();
    if(spectral_v4_config_.corridor_compression){
        if(!CompressDegreeTwoChains(raw_snapshot, preserved_ids, snapshot, reason)){
            return false;
        }
    }
    else{
        snapshot = std::move(raw_snapshot);
    }
    last_compressed_spectral_node_count_ = snapshot.nodes.size();

    if(spectral_config_.max_spectral_nodes != 0U &&
       snapshot.nodes.size() > spectral_config_.max_spectral_nodes){
        std::ostringstream stream;
        stream << "compressed support graph has " << snapshot.nodes.size()
               << " nodes, over configured limit "
               << spectral_config_.max_spectral_nodes;
        reason = stream.str();
        return false;
    }
    if(spectral_config_.weight_mode ==
       SpectralWeightMode::DISTANCE_CLEARANCE_BETWEENNESS){
        return ComputeEdgeBetweenness(snapshot, reason);
    }
    return true;
}

// ── V4.3: Landmark-based skeleton downsampling ──

bool MultiDtgPlus::SelectSkeletonLandmarks(
    const GlobalRouteContext &context,
    SpectralGraphSnapshot &snapshot, std::string &reason){
    const size_t n = context.active_hnodes.size();
    const int max_nodes = spectral_v4_config_.skeleton_max_nodes;
    const int landmark_count = spectral_v4_config_.skeleton_landmark_count;

    if(static_cast<int>(n) <= max_nodes){
        // No downsampling needed — build full support graph.
        return BuildSparseSupportSpectralGraph(context, snapshot, reason);
    }

    // Extract positions and select landmarks via farthest-point sampling.
    std::vector<Eigen::Vector3d> positions;
    positions.reserve(n);
    for(const auto &h : context.active_hnodes){
        positions.push_back(h->pos_);
    }

    const size_t k = std::min(static_cast<size_t>(landmark_count), n);
    const std::vector<size_t> landmark_indices = FarthestPointSampling(positions, k);

    // Build a reduced context with only landmark nodes as "active".
    GlobalRouteContext landmark_context;
    landmark_context.seed_hnodes = context.seed_hnodes;
    landmark_context.seed_distances = context.seed_distances;
    landmark_context.seed_paths = context.seed_paths;

    // Landmarks plus any seed nodes that are active boundaries
    std::unordered_set<uint32_t> landmark_ids;
    for(const size_t idx : landmark_indices){
        if(idx < context.active_hnodes.size()){
            const auto &h = context.active_hnodes[idx];
            if(h != nullptr) landmark_ids.insert(h->id_);
        }
    }
    // Always include seeds that are active boundaries
    for(const auto &h : context.seed_hnodes){
        if(h != nullptr && IsActiveBoundary(h)){
            landmark_ids.insert(h->id_);
        }
    }

    for(const auto &h : context.active_hnodes){
        if(h != nullptr && landmark_ids.count(h->id_)){
            landmark_context.active_hnodes.push_back(h);
        }
    }

    // Build root path entries for landmark nodes only
    for(const auto &h : landmark_context.active_hnodes){
        const auto it = context.active_index.find(h->id_);
        if(it != context.active_index.end() && it->second < context.root_paths.size()){
            landmark_context.root_paths.push_back(context.root_paths[it->second]);
        }
        else{
            landmark_context.root_paths.push_back({1e8, {}});
        }
        landmark_context.active_index[h->id_] = landmark_context.active_hnodes.size() - 1;
    }

    if(landmark_context.active_hnodes.empty()){
        reason = "landmark downsampling produced an empty active set";
        return false;
    }

    ROS_WARN("[Spectral-EDEN V4.3] Landmark downsampling: %zu → %zu nodes",
             n, landmark_context.active_hnodes.size());

    return BuildSparseSupportSpectralGraph(landmark_context, snapshot, reason);
}

bool MultiDtgPlus::BuildSpectralGraphSnapshot(
    const GlobalRouteContext &context, SpectralGraphSnapshot &snapshot,
    std::string &reason){
    // Check if landmark downsampling is needed
    if(static_cast<int>(context.active_hnodes.size()) >
       spectral_v4_config_.skeleton_max_nodes){
        return SelectSkeletonLandmarks(context, snapshot, reason);
    }
    return BuildSparseSupportSpectralGraph(context, snapshot, reason);
}

// ── Corridor compression (unchanged) ──

bool MultiDtgPlus::CompressDegreeTwoChains(
    const SpectralGraphSnapshot &input,
    const std::unordered_set<uint32_t> &preserved_h_ids,
    SpectralGraphSnapshot &output, std::string &reason) const{
    output = SpectralGraphSnapshot();
    output.graph_version = input.graph_version;
    if(input.nodes.empty() || input.edges.empty()){
        reason = "cannot compress an empty spectral support graph";
        return false;
    }

    struct Adjacent{
        size_t node = 0;
        size_t edge = 0;
    };
    std::unordered_map<uint32_t, size_t> index_by_id;
    index_by_id.reserve(input.nodes.size());
    for(size_t i = 0; i < input.nodes.size(); ++i){
        index_by_id[input.nodes[i].h_id] = i;
    }
    vector<vector<Adjacent>> adjacency(input.nodes.size());
    for(size_t i = 0; i < input.edges.size(); ++i){
        const auto from = index_by_id.find(input.edges[i].from_h_id);
        const auto to = index_by_id.find(input.edges[i].to_h_id);
        if(from == index_by_id.end() || to == index_by_id.end()){
            reason = "corridor compression found an unknown edge endpoint";
            return false;
        }
        adjacency[from->second].push_back({to->second, i});
        adjacency[to->second].push_back({from->second, i});
    }

    vector<unsigned char> preserve(input.nodes.size(), 0U);
    for(size_t i = 0; i < input.nodes.size(); ++i){
        if(input.nodes[i].active_anchor ||
           preserved_h_ids.count(input.nodes[i].h_id) != 0U ||
           adjacency[i].size() != 2U){
            preserve[i] = 1U;
        }
    }

    vector<unsigned char> component_seen(input.nodes.size(), 0U);
    for(size_t seed = 0; seed < input.nodes.size(); ++seed){
        if(component_seen[seed] != 0U) continue;
        vector<size_t> component;
        vector<size_t> open(1U, seed);
        component_seen[seed] = 1U;
        bool has_preserved_node = false;
        while(!open.empty()){
            const size_t current = open.back();
            open.pop_back();
            component.emplace_back(current);
            has_preserved_node = has_preserved_node || preserve[current] != 0U;
            for(const Adjacent &next : adjacency[current]){
                if(component_seen[next.node] == 0U){
                    component_seen[next.node] = 1U;
                    open.emplace_back(next.node);
                }
            }
        }
        if(!has_preserved_node){
            std::sort(component.begin(), component.end(), [&](size_t lhs, size_t rhs){
                return input.nodes[lhs].h_id < input.nodes[rhs].h_id;
            });
            const size_t break_count = std::min<size_t>(3U, component.size());
            for(size_t i = 0; i < break_count; ++i) preserve[component[i]] = 1U;
        }
    }

    struct CorridorChain{
        size_t start = 0U;
        size_t end = 0U;
        vector<size_t> nodes;
        vector<size_t> edges;
    };
    vector<CorridorChain> chains;
    vector<unsigned char> visited_edge(input.edges.size(), 0U);
    for(size_t start = 0; start < input.nodes.size(); ++start){
        if(preserve[start] == 0U) continue;
        for(const Adjacent &first : adjacency[start]){
            if(visited_edge[first.edge] != 0U) continue;
            CorridorChain chain;
            chain.start = start;
            chain.nodes.emplace_back(start);
            size_t previous = start;
            size_t current = first.node;
            size_t current_edge = first.edge;

            while(true){
                if(visited_edge[current_edge] != 0U){
                    reason = "corridor compression encountered an already consumed chain edge";
                    return false;
                }
                visited_edge[current_edge] = 1U;
                chain.edges.emplace_back(current_edge);
                chain.nodes.emplace_back(current);
                if(preserve[current] != 0U) break;
                if(adjacency[current].size() != 2U){
                    reason = "non-preserved corridor node does not have degree two";
                    return false;
                }
                const Adjacent next = adjacency[current][0].node == previous
                    ? adjacency[current][1] : adjacency[current][0];
                previous = current;
                current = next.node;
                current_edge = next.edge;
            }
            chain.end = current;
            chains.emplace_back(std::move(chain));
        }
    }

    if(std::find(visited_edge.begin(), visited_edge.end(), 0U) != visited_edge.end()){
        reason = "corridor compression left an unanchored topology component";
        return false;
    }

    std::unordered_set<uint32_t> output_node_ids;
    auto add_output_node = [&](size_t index){
        if(output_node_ids.insert(input.nodes[index].h_id).second){
            output.nodes.emplace_back(input.nodes[index]);
        }
    };
    for(size_t i = 0; i < input.nodes.size(); ++i){
        if(preserve[i] != 0U) add_output_node(i);
    }

    std::map<uint64_t, vector<size_t>> chain_groups;
    for(size_t i = 0; i < chains.size(); ++i){
        const uint32_t from = input.nodes[chains[i].start].h_id;
        const uint32_t to = input.nodes[chains[i].end].h_id;
        const uint64_t key = (static_cast<uint64_t>(std::min(from, to)) << 32U) |
            static_cast<uint64_t>(std::max(from, to));
        chain_groups[key].emplace_back(i);
    }

    std::unordered_set<uint64_t> output_edge_keys;
    auto emit_segment = [&](const CorridorChain &chain, size_t begin,
                            size_t end){
        if(begin >= end || end > chain.edges.size()) return false;
        const uint32_t from = input.nodes[chain.nodes[begin]].h_id;
        const uint32_t to = input.nodes[chain.nodes[end]].h_id;
        if(from == to) return false;
        const uint64_t key = (static_cast<uint64_t>(std::min(from, to)) << 32U) |
            static_cast<uint64_t>(std::max(from, to));
        if(!output_edge_keys.insert(key).second) return false;
        double length = 0.0;
        double clearance = std::numeric_limits<double>::infinity();
        double betweenness = 0.0;
        for(size_t i = begin; i < end; ++i){
            const SpectralEdgeInput &edge = input.edges[chain.edges[i]];
            length += edge.length;
            if(std::isfinite(edge.clearance)){
                clearance = std::min(clearance, edge.clearance);
            }
            if(std::isfinite(edge.betweenness)){
                betweenness = std::max(betweenness, edge.betweenness);
            }
        }
        SpectralEdgeInput compressed(from, to, length);
        compressed.clearance = std::isfinite(clearance)
            ? clearance : std::numeric_limits<double>::quiet_NaN();
        compressed.betweenness = betweenness;
        output.edges.emplace_back(compressed);
        return true;
    };

    for(const auto &group : chain_groups){
        for(const size_t chain_index : group.second){
            const CorridorChain &chain = chains[chain_index];
            const size_t edge_count = chain.edges.size();
            bool emitted = false;
            if(chain.start == chain.end){
                if(edge_count < 3U){
                    reason = "corridor compression found a degenerate cycle";
                    return false;
                }
                const size_t first_break = std::max<size_t>(1U, edge_count / 3U);
                const size_t second_break = std::min(edge_count - 1U,
                    std::max(first_break + 1U, (2U * edge_count) / 3U));
                add_output_node(chain.nodes[first_break]);
                add_output_node(chain.nodes[second_break]);
                emitted = emit_segment(chain, 0U, first_break) &&
                    emit_segment(chain, first_break, second_break) &&
                    emit_segment(chain, second_break, edge_count);
            }
            else if(group.second.size() > 1U && edge_count > 1U){
                const size_t split = edge_count / 2U;
                add_output_node(chain.nodes[split]);
                emitted = emit_segment(chain, 0U, split) &&
                    emit_segment(chain, split, edge_count);
            }
            else{
                emitted = emit_segment(chain, 0U, edge_count);
            }
            if(!emitted){
                reason = "corridor compression produced a duplicate support edge";
                return false;
            }
        }
    }
    if(output.edges.empty()){
        reason = "corridor compression removed every usable support edge";
        return false;
    }
    return true;
}

// ── Region matching and persistence (simplified for V4) ──

void MultiDtgPlus::MatchAndUpdatePersistentRegions(
    const SpectralResult &result, const GlobalRouteContext &context){
    if(!result.has_valid_cut()) return;

    std::unordered_set<uint32_t> clusters[2];
    for(size_t i = 0; i < result.h_ids.size() && i < result.labels.size(); ++i){
        if(result.labels[i] == 0 || result.labels[i] == 1){
            clusters[result.labels[i]].insert(result.h_ids[i]);
        }
    }
    if(clusters[0].empty() || clusters[1].empty()) return;

    int cluster_region[2] = {-1, -1};
    std::unordered_set<int> consumed_old_regions;
    bool created_new_region = false;

    // Preserve active region via Jaccard match
    if(active_region_id_ >= 0){
        const auto active_old = regions_.find(active_region_id_);
        if(active_old != regions_.end()){
            const double overlap0 = Jaccard(clusters[0], active_old->second.node_ids);
            const double overlap1 = Jaccard(clusters[1], active_old->second.node_ids);
            const double best_overlap = std::max(overlap0, overlap1);
            if(best_overlap >= spectral_v4_config_.region_match_threshold){
                const int chosen = overlap1 > overlap0 ? 1 : 0;
                cluster_region[chosen] = active_region_id_;
                consumed_old_regions.insert(active_region_id_);
            }
        }
    }

    for(int cluster = 0; cluster < 2; ++cluster){
        if(cluster_region[cluster] >= 0) continue;
        double best_score = -1.0;
        int best_region = -1;
        for(const auto &old_region : regions_){
            if(consumed_old_regions.count(old_region.first) != 0U) continue;
            const double score = Jaccard(clusters[cluster], old_region.second.node_ids);
            if(score > best_score){
                best_score = score;
                best_region = old_region.first;
            }
        }
        if(best_region >= 0 && best_score >= spectral_v4_config_.region_match_threshold){
            cluster_region[cluster] = best_region;
            consumed_old_regions.insert(best_region);
        }
        else{
            cluster_region[cluster] = next_region_id_++;
            created_new_region = true;
        }
    }

    std::unordered_map<int, RegionState> updated_regions;
    std::unordered_map<uint32_t, int> updated_node_regions;

    for(int cluster = 0; cluster < 2; ++cluster){
        RegionState region;
        const auto old = regions_.find(cluster_region[cluster]);
        if(old != regions_.end()) region = old->second;
        region.id = cluster_region[cluster];
        region.node_ids.clear();
        region.active_anchor_ids.clear();
        region.spectral_center = 0.0;
        region.last_seen_spectral_epoch = spectral_epoch_;
        updated_regions[region.id] = region;
    }

    size_t center_counts[2] = {0U, 0U};
    for(size_t i = 0; i < result.h_ids.size() && i < result.labels.size(); ++i){
        const int raw_cluster = result.labels[i];
        if(raw_cluster != 0 && raw_cluster != 1) continue;
        int region_id = cluster_region[raw_cluster];
        updated_node_regions[result.h_ids[i]] = region_id;
        updated_regions[region_id].node_ids.insert(result.h_ids[i]);
        updated_regions[region_id].spectral_center +=
            result.fiedler(static_cast<Eigen::Index>(i));
        ++center_counts[region_id == cluster_region[0] ? 0 : 1];
    }
    for(int cluster = 0; cluster < 2; ++cluster){
        RegionState &region = updated_regions[cluster_region[cluster]];
        const size_t count = center_counts[cluster];
        if(count > 0U) region.spectral_center /= static_cast<double>(count);
    }

    for(const auto &h : context.active_hnodes){
        if(h == nullptr) continue;
        const auto region = updated_node_regions.find(h->id_);
        if(region != updated_node_regions.end()){
            updated_regions[region->second].active_anchor_ids.insert(h->id_);
        }
    }

    // Track partition change type
    if(regions_.empty()){
        last_partition_change_ = PartitionChangeType::CREATED;
    }
    else if(created_new_region){
        last_partition_change_ = PartitionChangeType::RESET;
    }
    else{
        last_partition_change_ = PartitionChangeType::NONE;
    }

    regions_.swap(updated_regions);
    h_region_ids_ = updated_node_regions;
    previous_region_ids_ = updated_node_regions;
}

void MultiDtgPlus::UpdateRegionExecutionState(const GlobalRouteContext &context){
    if(regions_.empty()) return;

    for(auto &entry : regions_) entry.second.active_anchor_ids.clear();
    std::unordered_map<int, double> min_root_distance;
    for(size_t i = 0; i < context.active_hnodes.size(); ++i){
        const h_ptr &h = context.active_hnodes[i];
        if(h == nullptr) continue;
        const auto region = h_region_ids_.find(h->id_);
        if(region == h_region_ids_.end() || regions_.count(region->second) == 0U) continue;
        if(!HasEffectiveFrontier(h, region->second)) continue;
        regions_[region->second].active_anchor_ids.insert(h->id_);
        const double distance = i < context.root_paths.size()
            ? context.root_paths[i].first : std::numeric_limits<double>::infinity();
        auto found = min_root_distance.find(region->second);
        if(found == min_root_distance.end() || distance < found->second){
            min_root_distance[region->second] = distance;
        }
    }

    // Update frontier epoch tracking
    const bool new_frontier_epoch = last_region_frontier_epoch_ != frontier_epoch_;
    if(new_frontier_epoch){
        last_region_frontier_epoch_ = frontier_epoch_;
        for(auto &entry : regions_){
            RegionState &region = entry.second;
            if(region.id == active_region_id_){
                region.empty_streak = region.active_anchor_ids.empty()
                    ? region.empty_streak + 1 : 0;
            }
        }
    }

    // Mark active region as DONE when its frontiers are exhausted
    if(active_region_id_ >= 0){
        auto active = regions_.find(active_region_id_);
        if(active == regions_.end()){
            active_region_id_ = -1;
        }
        else if(active->second.empty_streak >=
                std::max(1, spectral_v4_config_.region_done_cycles)){
            active->second.exec_state = RegionExecState::DONE;
            missed_region_set_.erase(active_region_id_);
            active_region_id_ = -1;
        }
    }

    auto region_is_viable = [&](int id){
        const auto region = regions_.find(id);
        return region != regions_.end() && !region->second.active_anchor_ids.empty() &&
               region->second.exec_state != RegionExecState::DONE;
    };

    // Select next active region
    if(active_region_id_ < 0 && partition_valid_){
        while(!missed_regions_.empty() && !region_is_viable(missed_regions_.front())){
            missed_region_set_.erase(missed_regions_.front());
            missed_regions_.pop_front();
        }
        if(!missed_regions_.empty()){
            active_region_id_ = missed_regions_.front();
            missed_region_set_.erase(active_region_id_);
            missed_regions_.pop_front();
        }
        else{
            double best_distance = std::numeric_limits<double>::infinity();
            for(const auto &distance : min_root_distance){
                if(region_is_viable(distance.first) && distance.second < best_distance){
                    best_distance = distance.second;
                    active_region_id_ = distance.first;
                }
            }
        }
    }

    // Rebuild missed-region queue
    std::deque<int> rebuilt;
    std::unordered_set<int> rebuilt_set;
    for(const int id : missed_regions_){
        if(id != active_region_id_ && region_is_viable(id) && rebuilt_set.insert(id).second){
            rebuilt.push_back(id);
        }
    }
    vector<int> append_ids;
    for(const auto &entry : regions_){
        if(entry.first != active_region_id_ && region_is_viable(entry.first) &&
           rebuilt_set.count(entry.first) == 0U){
            append_ids.emplace_back(entry.first);
        }
    }
    std::sort(append_ids.begin(), append_ids.end(), [&](int lhs, int rhs){
        return regions_.at(lhs).spectral_center < regions_.at(rhs).spectral_center;
    });
    for(const int id : append_ids){
        rebuilt.push_back(id);
        rebuilt_set.insert(id);
    }
    missed_regions_.swap(rebuilt);
    missed_region_set_.swap(rebuilt_set);

    for(auto &entry : regions_){
        if(entry.second.exec_state == RegionExecState::DONE) continue;
        entry.second.exec_state = entry.first == active_region_id_
            ? RegionExecState::ACTIVE : RegionExecState::DORMANT;
    }
}

void MultiDtgPlus::PublishSpectralStateToHnodes(const SpectralResult &result){
    for(const auto &h : H_list_){
        if(h == nullptr) continue;
        const auto fiedler = result.fiedler_by_h_id.find(h->id_);
        if(fiedler == result.fiedler_by_h_id.end()) continue;
        h->fiedler_val_ = fiedler->second;
        const auto region = h_region_ids_.find(h->id_);
        h->spectral_region_id_ = region == h_region_ids_.end() ? -1 : region->second;
        h->spectral_epoch_ = spectral_epoch_;
    }
}

// ── Region quotient target selection (V4.1 core) ──

int MultiDtgPlus::RegionForHnode(const h_ptr &h) const{
    if(h == nullptr) return -1;
    const auto region = h_region_ids_.find(h->id_);
    return region == h_region_ids_.end() ? -1 : region->second;
}

double MultiDtgPlus::ContextRouteDistance(
    const GlobalRouteContext &context, const h_ptr &from,
    const h_ptr &to) const{
    if(to == nullptr) return std::numeric_limits<double>::infinity();
    const auto to_index = context.active_index.find(to->id_);
    if(to_index == context.active_index.end()){
        return std::numeric_limits<double>::infinity();
    }
    if(from == nullptr){
        return to_index->second < context.root_paths.size()
            ? context.root_paths[to_index->second].first
            : std::numeric_limits<double>::infinity();
    }
    if(from->id_ == to->id_) return 0.0;
    const auto from_index = context.active_index.find(from->id_);
    if(from_index == context.active_index.end()){
        return std::numeric_limits<double>::infinity();
    }
    // Use root-path star as upper bound for inter-anchor distance
    if(from_index->second < context.root_paths.size() &&
       to_index->second < context.root_paths.size()){
        return context.root_paths[from_index->second].first +
               context.root_paths[to_index->second].first;
    }
    return std::numeric_limits<double>::infinity();
}

bool MultiDtgPlus::SelectTargetFromQuotientGraph(
    const GlobalRouteContext &context, h_ptr &target,
    std::string &reason) const{
    target = nullptr;
    if(!partition_valid_ || regions_.empty()){
        reason = "no valid partition for quotient-graph selection";
        return false;
    }

    // Find the closest region with active anchors
    int target_region = active_region_id_;
    if(target_region < 0 || regions_.count(target_region) == 0U){
        double best_distance = std::numeric_limits<double>::infinity();
        for(const auto &entry : regions_){
            if(entry.second.exec_state == RegionExecState::DONE) continue;
            if(entry.second.active_anchor_ids.empty()) continue;
            // Find the closest anchor in this region
            for(const auto &h : context.active_hnodes){
                if(h == nullptr) continue;
                if(entry.second.active_anchor_ids.count(h->id_)){
                    const double d = ContextRouteDistance(context, nullptr, h);
                    if(d < best_distance){
                        best_distance = d;
                        target_region = entry.first;
                    }
                    break;
                }
            }
        }
    }

    if(target_region < 0){
        reason = "no viable region in quotient graph";
        return false;
    }

    // Select the best anchor within the target region
    double best_score = -std::numeric_limits<double>::infinity();
    for(const auto &h : context.active_hnodes){
        if(h == nullptr) continue;
        if(RegionForHnode(h) != target_region) continue;
        if(!HasEffectiveFrontier(h, target_region)) continue;
        const double d = ContextRouteDistance(context, nullptr, h);
        if(!std::isfinite(d) || d <= 0.0) continue;
        const double gain = static_cast<double>(h->hf_edges_.size());
        const double score = gain / d;
        if(score > best_score){
            best_score = score;
            target = h;
        }
    }

    if(target == nullptr){
        reason = "no reachable anchor in target region " + std::to_string(target_region);
        return false;
    }

    reason = "quotient-graph selected target in region " + std::to_string(target_region);
    return true;
}

// ── V4.2: Local APPR fallback ──

bool MultiDtgPlus::ComputeLocalAPPRFallback(
    const GlobalRouteContext &context,
    const Eigen::Vector3d &robot_pos, h_ptr &target,
    std::string &reason) const{
    target = nullptr;

    // Use seed H-nodes as the local neighborhood center.
    // APPR would normally spread from the robot's closest graph nodes, but
    // the seed nodes already represent the reachable local neighborhood.
    if(context.seed_hnodes.empty()){
        reason = "no seed nodes for local APPR fallback";
        return false;
    }

    // Collect all H-nodes within the local neighborhood (already computed
    // via ParallelDijkstra in context.active_hnodes, sorted by root distance).
    // Select the top-k closest active anchors.
    const int top_k = spectral_v4_config_.local_frontier_top_k;
    vector<pair<double, h_ptr>> local_candidates;
    local_candidates.reserve(context.active_hnodes.size());
    for(const auto &h : context.active_hnodes){
        if(h == nullptr || !HasEffectiveFrontier(h)) continue;
        const double d = ContextRouteDistance(context, nullptr, h);
        if(std::isfinite(d) && d > 0.0){
            local_candidates.emplace_back(d, h);
        }
    }

    if(local_candidates.empty()){
        reason = "no local candidates for APPR fallback";
        return false;
    }

    // Sort by distance and keep top-k
    std::sort(local_candidates.begin(), local_candidates.end(),
        [](const pair<double, h_ptr> &a, const pair<double, h_ptr> &b){
            return a.first < b.first;
        });

    const size_t keep = std::min(local_candidates.size(),
        static_cast<size_t>(top_k));
    local_candidates.resize(keep);

    // Among the top-k closest, pick the one with the best gain/distance ratio
    double best_score = -std::numeric_limits<double>::infinity();
    for(const auto &candidate : local_candidates){
        const double gain = static_cast<double>(candidate.second->hf_edges_.size());
        const double score = std::log1p(gain) / std::max(candidate.first, 1.0);
        if(score > best_score){
            best_score = score;
            target = candidate.second;
        }
    }

    if(target == nullptr){
        reason = "local APPR fallback found no viable target";
        return false;
    }

    reason = "local APPR fallback selected target at distance "
        + std::to_string(ContextRouteDistance(context, nullptr, target));
    return true;
}

// ── Async spectral job management ──

bool MultiDtgPlus::NeedSpectralUpdate(const GlobalRouteContext &context,
    double now) const{
    if(!spectral_v4_config_.enabled || spectral_job_running_){
        return false;
    }
    const double elapsed = last_spectral_update_time_ < 0.0
        ? std::numeric_limits<double>::infinity()
        : now - last_spectral_update_time_;
    if(elapsed + spectral_config_.numeric_epsilon <
       spectral_v4_config_.spectral_min_update_interval){
        return false;
    }
    if(!spectral_graph_eligible_ || !last_spectral_result_.success()){
        return true;
    }
    if(elapsed >= spectral_v4_config_.spectral_max_update_interval){
        return true;
    }
    const size_t anchor_delta = context.active_hnodes.size() > last_submitted_anchor_count_
        ? context.active_hnodes.size() - last_submitted_anchor_count_
        : last_submitted_anchor_count_ - context.active_hnodes.size();
    return anchor_delta >= static_cast<size_t>(spectral_v4_config_.dirty_node_changes) ||
        frontier_changes_since_submit_ >= static_cast<uint64_t>(
            spectral_v4_config_.dirty_node_changes) ||
        topology_changes_since_submit_ >= static_cast<uint64_t>(
            spectral_v4_config_.dirty_edge_changes);
}

void MultiDtgPlus::SubmitSpectralJobAsync(
    const GlobalRouteContext &context, double now, std::string &reason){
    if(spectral_job_running_) return;
    const double build_start = ros::WallTime::now().toSec();
    SpectralGraphSnapshot snapshot;
    if(!BuildSpectralGraphSnapshot(context, snapshot, reason)){
        spectral_graph_eligible_ = false;
        last_spectral_update_time_ = now;
        return;
    }
    const double build_ms = (ros::WallTime::now().toSec() - build_start) * 1000.0;
    if(snapshot.nodes.size() < spectral_config_.min_spectral_nodes){
        std::ostringstream stream;
        stream << "support graph has only " << snapshot.nodes.size()
               << " nodes; waiting for more topology";
        reason = stream.str();
        spectral_graph_eligible_ = false;
        last_spectral_update_time_ = now;
        return;
    }

    // Check against skeleton budget
    if(static_cast<int>(snapshot.nodes.size()) >
       spectral_v4_config_.skeleton_max_nodes){
        std::ostringstream stream;
        stream << "support graph has " << snapshot.nodes.size()
               << " nodes after compression, over skeleton budget "
               << spectral_v4_config_.skeleton_max_nodes
               << "; will use local fallback";
        reason = stream.str();
        spectral_graph_eligible_ = false;
        last_spectral_update_time_ = now;
        return;
    }

    spectral_graph_eligible_ = true;

    const uint64_t submitted_dtg = dtg_version_;
    const uint64_t submitted_frontier = frontier_version_;
    const size_t raw_nodes = last_raw_spectral_node_count_;
    SpectralRouter router = spectral_router_;
    FiedlerHistory history = previous_fiedler_;
    spectral_future_ = std::async(std::launch::async,
        [router, snapshot, history, submitted_dtg, submitted_frontier,
         raw_nodes, now, build_ms]() mutable {
            SpectralAsyncOutput output;
            output.snapshot = std::move(snapshot);
            output.dtg_version = submitted_dtg;
            output.frontier_version = submitted_frontier;
            output.raw_node_count = raw_nodes;
            output.submitted_time = now;
            output.snapshot_build_ms = build_ms;
            output.result = router.Solve(output.snapshot, history);
            return output;
        });
    spectral_job_running_ = true;
    ++submitted_spectral_jobs_;
    last_submitted_dtg_version_ = submitted_dtg;
    last_submitted_frontier_version_ = submitted_frontier;
    last_submitted_anchor_count_ = context.active_hnodes.size();
    topology_changes_since_submit_ = 0;
    frontier_changes_since_submit_ = 0;
    last_spectral_update_time_ = now;
    spectral_dirty_ = false;
}

bool MultiDtgPlus::ConsumeSpectralResult(
    const GlobalRouteContext &context, double now, std::string &reason){
    if(!spectral_job_running_) return false;
    const std::future_status status = spectral_future_.wait_for(
        std::chrono::milliseconds(0));
    if(status != std::future_status::ready) return false;

    SpectralAsyncOutput output = spectral_future_.get();
    spectral_job_running_ = false;
    if(output.dtg_version != dtg_version_ ||
       output.frontier_version != frontier_version_){
        ++stale_spectral_results_;
        reason = "discarded stale asynchronous spectral result";
        return false;
    }
    if(!output.result.success()){
        spectral_graph_eligible_ = false;
        reason = std::string(SpectralSolveStatusName(output.result.status)) +
            ": " + output.result.diagnostics.reason;
        return false;
    }

    ++spectral_epoch_;
    output.result.diagnostics.total_time_ms += output.snapshot_build_ms;
    last_spectral_result_ = std::move(output.result);
    last_spectral_snapshot_ = std::move(output.snapshot);
    last_raw_spectral_node_count_ = output.raw_node_count;
    last_compressed_spectral_node_count_ = last_spectral_snapshot_.nodes.size();

    // Update EMA
    lambda2_ema_ = UpdateLambda2Ema(last_spectral_result_.diagnostics.lambda2,
        lambda2_ema_initialized_ ? lambda2_ema_
                                 : std::numeric_limits<double>::quiet_NaN(),
        spectral_v4_config_.lambda2_ema_alpha);
    lambda2_ema_initialized_ = std::isfinite(lambda2_ema_);

    if(last_spectral_result_.has_valid_cut()){
        // Accept the result if Ncut is below threshold
        if(last_spectral_result_.diagnostics.ncut <=
           spectral_v4_config_.ncut_threshold){
            stable_spectral_result_ = last_spectral_result_;
            last_spectral_dtg_version_ = output.dtg_version;
            last_spectral_frontier_version_ = output.frontier_version;
            previous_fiedler_ = stable_spectral_result_.fiedler_by_h_id;
            MatchAndUpdatePersistentRegions(stable_spectral_result_, context);
            PublishSpectralStateToHnodes(stable_spectral_result_);
            partition_valid_ = true;
            reason = "spectral cut accepted (Ncut="
                + std::to_string(last_spectral_result_.diagnostics.ncut) + ")";
        }
        else{
            partition_valid_ = false;
            reason = "spectral cut rejected: Ncut "
                + std::to_string(last_spectral_result_.diagnostics.ncut)
                + " > threshold " + std::to_string(spectral_v4_config_.ncut_threshold);
        }
    }
    else{
        partition_valid_ = false;
        reason = "spectral solve found no valid cut";
    }

    UpdateRegionExecutionState(context);
    return true;
}

// ── Helpers ──

bool MultiDtgPlus::HasEffectiveFrontier(
    const h_ptr &h, int region_id) const{
    if(h == nullptr || EROI_ == nullptr) return false;
    for(const hfe_ptr &edge : h->hf_edges_){
        if(edge == nullptr || edge->tail_n_ == nullptr ||
           !std::isfinite(edge->length_) || edge->length_ >= kBlockedEdgeDistance) continue;
        const uint32_t fid = edge->tail_n_->fid_;
        const uint8_t vid = edge->tail_n_->vid_;
        if(fid >= EROI_->EROI_.size()) continue;
        const auto &frontier = EROI_->EROI_[fid];
        if(frontier.f_state_ != 1U || vid >= frontier.local_vps_.size() ||
           frontier.local_vps_[vid] != 1U ||
           static_cast<double>(frontier.unknown_num_) <=
               spectral_v4_config_.frontier_expected_gain_eps) continue;
        if(region_id < 0) return true;
        // For region-specific check, verify the frontier "belongs" to this region
        // (same active anchor set membership)
        const auto h_region = h_region_ids_.find(h->id_);
        if(h_region != h_region_ids_.end() && h_region->second == region_id) return true;
    }
    // If no specific region filter, any frontier counts
    if(region_id < 0 && !h->hf_edges_.empty()) return true;
    return false;
}

size_t MultiDtgPlus::EffectiveFrontierCount(double *total_gain) const{
    if(total_gain != nullptr) *total_gain = 0.0;
    if(EROI_ == nullptr) return 0U;
    size_t count = 0U;
    for(size_t fid = 0; fid < EROI_->EROI_.size(); ++fid){
        const auto &frontier = EROI_->EROI_[fid];
        if(frontier.f_state_ != 1U) continue;
        bool alive = false;
        for(size_t vid = 0; vid < frontier.local_vps_.size(); ++vid){
            if(frontier.local_vps_[vid] == 1U){
                alive = true;
                break;
            }
        }
        if(!alive) continue;
        if(static_cast<double>(frontier.unknown_num_) <=
           spectral_v4_config_.frontier_expected_gain_eps) continue;
        ++count;
        if(total_gain != nullptr) *total_gain += std::max(0, frontier.unknown_num_);
    }
    return count;
}

h_ptr MultiDtgPlus::ResolveSpectralCandidateH(const hfe_ptr &edge,
    const Eigen::Vector3d &viewpoint_pos){
    if(edge != nullptr && edge->head_n_ != nullptr) return edge->head_n_;
    if(LRM_ == nullptr) return nullptr;
    const auto local_node = LRM_->GlobalPos2LocalNode(viewpoint_pos);
    if(local_node == nullptr || local_node == LRM_->Outnode_ ||
       local_node->root_id_ == 0U) return nullptr;
    h_ptr candidate;
    if(!FindHnode(viewpoint_pos, local_node->root_id_, candidate)) return nullptr;
    return candidate;
}

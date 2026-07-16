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
}

double MultiDtgPlus::EstimatePathClearance(
    const list<Eigen::Vector3d> &path) const{
    if(path.empty() || BM_ == nullptr || LRM_ == nullptr) return 0.0;

    const double maximum = spectral_config_.clearance_reference;
    const Eigen::Vector3d robot_size = LRM_->GetRobotSize();
    const size_t max_samples = static_cast<size_t>(
        std::max(2, spectral_exec_config_.clearance_max_samples));
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
        for(int step = 0; step < spectral_exec_config_.clearance_binary_steps; ++step){
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

bool MultiDtgPlus::BuildActiveCompleteSpectralGraph(
    const GlobalRouteContext &context, SpectralGraphSnapshot &snapshot,
    std::string &reason){
    snapshot = SpectralGraphSnapshot();
    snapshot.graph_version = dtg_version_;
    for(const auto &h : context.active_hnodes){
        if(h == nullptr) continue;
        snapshot.nodes.emplace_back(h->id_, true);
    }
    for(size_t i = 0; i < context.active_hnodes.size(); ++i){
        for(size_t j = i + 1; j < context.active_hnodes.size(); ++j){
            const double length = context.distance_matrix(
                static_cast<Eigen::Index>(i + 1), static_cast<Eigen::Index>(j + 1));
            if(!std::isfinite(length) || length <= 0.0 || length >= kUnreachableDistance){
                reason = "active-region distance matrix contains an invalid entry";
                return false;
            }
            SpectralEdgeInput spectral_edge(context.active_hnodes[i]->id_,
                context.active_hnodes[j]->id_, length);
            if(spectral_config_.weight_mode != SpectralWeightMode::DISTANCE){
                const auto cached = h_dist_map_.find(HPairKey(
                    context.active_hnodes[i]->id_, context.active_hnodes[j]->id_));
                if(cached == h_dist_map_.end() || cached->second.dtg_version != dtg_version_){
                    reason = "complete-graph path clearance is absent from the live cache";
                    return false;
                }
                spectral_edge.clearance = CachedPathClearance(
                    context.active_hnodes[i], cached->second);
            }
            snapshot.edges.emplace_back(spectral_edge);
        }
    }
    if(spectral_config_.weight_mode ==
       SpectralWeightMode::DISTANCE_CLEARANCE_BETWEENNESS){
        return ComputeEdgeBetweenness(snapshot, reason);
    }
    return true;
}

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

    // Select only k relevant neighbours per active anchor.  Add an active-node
    // MST as a connectivity backbone: a symmetric kNN union alone can split
    // precisely at the long bottleneck that the spectral layer must inspect.
    std::unordered_set<uint64_t> support_pairs;
    auto pair_distance = [&](size_t i, size_t j){
        double distance = context.distance_matrix.rows() >
                static_cast<Eigen::Index>(i + 1) &&
            context.distance_matrix.cols() > static_cast<Eigen::Index>(j + 1)
            ? context.distance_matrix(static_cast<Eigen::Index>(i + 1),
                                      static_cast<Eigen::Index>(j + 1))
            : kUnreachableDistance;
        if(!std::isfinite(distance) || distance >= kUnreachableDistance){
            distance = (context.active_hnodes[i]->pos_ -
                        context.active_hnodes[j]->pos_).norm();
        }
        return distance;
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
            std::max(1, spectral_exec_config_.spectral_knn)));
        for(size_t k = 0; k < keep; ++k){
            const size_t j = neighbours[k].second;
            support_pairs.insert(HPairKey(context.active_hnodes[i]->id_,
                                          context.active_hnodes[j]->id_));
        }
    }

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

    // Materialize only selected kNN/backbone paths.  Grouping targets by start
    // preserves the existing batched Dijkstra implementation.
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

    // Retain the union of selected shortest paths, represented by real H-H
    // edges rather than all-pairs shortcut edges.
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

    // Root paths retain the live robot approach branches and their seed
    // endpoints.  Seeds are protected from corridor-chain compression.
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
        // A degree-two H-node with any frontier attachment is semantically an
        // anchor even if that frontier is currently deferred/quarantined.
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
                // Clearance also supplies the v2 bottleneck-confidence term,
                // even when the eigensolver itself uses distance-only weights.
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
    if(spectral_exec_config_.corridor_compression){
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

    // Every all-degree-two connected component is a ring.  Give it three
    // deterministic break points so its cycle cannot collapse to a self-loop
    // or a single parallel endpoint pair.
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
                // Parallel corridors must remain topologically distinct.  Keep
                // one real midpoint from each compressed chain instead of
                // silently discarding the longer route.
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

bool MultiDtgPlus::BuildSpectralGraphSnapshot(
    const GlobalRouteContext &context, SpectralGraphSnapshot &snapshot,
    std::string &reason){
    if(spectral_exec_config_.graph_mode == SpectralGraphMode::ACTIVE_COMPLETE){
        return BuildActiveCompleteSpectralGraph(context, snapshot, reason);
    }
    return BuildSparseSupportSpectralGraph(context, snapshot, reason);
}

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

    // Preserve the active execution ID only under the same Jaccard contract
    // as every other persistent region.  A single overlapping node is not
    // enough evidence to carry execution state across a large repartition.
    if(active_region_id_ >= 0){
        const auto active_old = regions_.find(active_region_id_);
        if(active_old != regions_.end()){
            const double overlap0 = Jaccard(clusters[0], active_old->second.node_ids);
            const double overlap1 = Jaccard(clusters[1], active_old->second.node_ids);
            const double best_overlap = std::max(overlap0, overlap1);
            if(best_overlap >= spectral_exec_config_.region_match_threshold){
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
        if(best_region >= 0 && best_score >= spectral_exec_config_.region_match_threshold){
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
    const double f_min = result.fiedler.size() == 0 ? 0.0 : result.fiedler.minCoeff();
    const double f_max = result.fiedler.size() == 0 ? 0.0 : result.fiedler.maxCoeff();
    const double hysteresis_width = spectral_exec_config_.label_hysteresis *
        std::max(f_max - f_min, spectral_config_.numeric_epsilon);

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
        if(std::abs(result.fiedler(static_cast<Eigen::Index>(i)) -
                    result.diagnostics.cut_threshold) <= hysteresis_width){
            const auto previous = previous_region_ids_.find(result.h_ids[i]);
            if(previous != previous_region_ids_.end() &&
               updated_regions.count(previous->second) != 0U){
                region_id = previous->second;
            }
        }
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

    if(regions_.empty()){
        last_partition_change_ = PartitionChangeType::CREATED;
    }
    else{
        bool split = false;
        for(const auto &old : regions_){
            if(!old.second.node_ids.empty() &&
               Jaccard(old.second.node_ids, clusters[0]) > 0.0 &&
               Jaccard(old.second.node_ids, clusters[1]) > 0.0){
                split = true;
                break;
            }
        }
        bool merge = false;
        for(int cluster = 0; cluster < 2; ++cluster){
            int overlapping_regions = 0;
            for(const auto &old : regions_){
                if(Jaccard(clusters[cluster], old.second.node_ids) > 0.0){
                    ++overlapping_regions;
                }
            }
            if(overlapping_regions > 1){
                merge = true;
                break;
            }
        }
        if(split) last_partition_change_ = PartitionChangeType::SPLIT;
        else if(merge) last_partition_change_ = PartitionChangeType::MERGE;
        else if(created_new_region) last_partition_change_ = PartitionChangeType::RESET;
        else last_partition_change_ = PartitionChangeType::NONE;
    }
    size_t shared_labels = 0U;
    size_t changed_labels = 0U;
    for(const auto &label : updated_node_regions){
        const auto previous = previous_region_ids_.find(label.first);
        if(previous == previous_region_ids_.end()) continue;
        ++shared_labels;
        if(previous->second != label.second) ++changed_labels;
    }
    last_label_change_rate_ = shared_labels == 0U ? 0.0 :
        static_cast<double>(changed_labels) / static_cast<double>(shared_labels);
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

    const bool new_frontier_epoch = last_region_frontier_epoch_ != frontier_epoch_;
    if(new_frontier_epoch){
        last_region_frontier_epoch_ = frontier_epoch_;
        for(auto &entry : regions_){
            RegionState &region = entry.second;
            region.last_counted_frontier_epoch = frontier_epoch_;
            if(region.id == active_region_id_){
                region.empty_streak = region.active_anchor_ids.empty()
                    ? region.empty_streak + 1 : 0;
            }
        }
    }

    if(active_region_id_ >= 0){
        auto active = regions_.find(active_region_id_);
        if(active == regions_.end()){
            active_region_id_ = -1;
        }
        else if(active->second.empty_streak >=
                std::max(1, spectral_exec_config_.region_done_cycles)){
            active->second.exec_state = RegionExecState::DONE;
            missed_region_set_.erase(active_region_id_);
            active_region_id_ = -1;
        }
    }

    auto region_is_viable = [&](int id){
        const auto region = regions_.find(id);
        return region != regions_.end() && !region->second.active_anchor_ids.empty() &&
               region->second.exec_state != RegionExecState::DONE &&
               region->second.exec_state != RegionExecState::STALE &&
               region->second.exec_state != RegionExecState::UNREACHABLE;
    };

    for(auto &entry : regions_){
        RegionState &region = entry.second;
        if(region.active_anchor_ids.empty()) continue;
        const bool frontier_reappeared = region.exec_state == RegionExecState::DONE;
        const bool topology_recovered = region.exec_state == RegionExecState::UNREACHABLE &&
            spectral_epoch_ > region.unreachable_spectral_epoch;
        if(frontier_reappeared || topology_recovered){
            region.exec_state = RegionExecState::DORMANT;
            region.empty_streak = 0;
            region.unreachable_streak = 0;
        }
    }

    if(active_region_id_ < 0 &&
       spectral_mode_state_.mode == SpectralMode::ACTIVE_SOFT && partition_valid_){
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

    // Preserve still-valid missed-region order, then append newly discovered
    // regions by increasing spectral center for deterministic backtracking.
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

bool MultiDtgPlus::NeedSpectralUpdate(const GlobalRouteContext &context,
    double now) const{
    if(!spectral_exec_config_.enabled || spectral_job_running_ ||
       late_stage_active_ || spectral_mode_state_.mode == SpectralMode::RECOVERY){
        return false;
    }
    const double elapsed = last_spectral_update_time_ < 0.0
        ? std::numeric_limits<double>::infinity()
        : now - last_spectral_update_time_;
    if(elapsed + spectral_config_.numeric_epsilon <
       spectral_exec_config_.spectral_min_update_interval){
        return false;
    }
    if((spectral_graph_eligible_ && !last_spectral_result_.success()) ||
       elapsed >= spectral_exec_config_.spectral_max_update_interval){
        return true;
    }
    const size_t anchor_delta = context.active_hnodes.size() > last_submitted_anchor_count_
        ? context.active_hnodes.size() - last_submitted_anchor_count_
        : last_submitted_anchor_count_ - context.active_hnodes.size();
    return anchor_delta >= static_cast<size_t>(spectral_exec_config_.dirty_node_changes) ||
        frontier_changes_since_submit_ >= static_cast<uint64_t>(
            spectral_exec_config_.dirty_node_changes) ||
        topology_changes_since_submit_ >= static_cast<uint64_t>(
            spectral_exec_config_.dirty_edge_changes);
}

void MultiDtgPlus::SubmitSpectralJobAsync(
    const GlobalRouteContext &context, double now, std::string &reason){
    if(spectral_job_running_) return;
    const double build_start = ros::WallTime::now().toSec();
    SpectralGraphSnapshot snapshot;
    if(!BuildSpectralGraphSnapshot(context, snapshot, reason)){
        spectral_fallback_this_cycle_ = true;
        spectral_graph_eligible_ = false;
        last_spectral_update_time_ = now;
        UpdateSpectralRuntimeMode(false, false, now);
        return;
    }
    const double build_ms = (ros::WallTime::now().toSec() - build_start) * 1000.0;
    if(snapshot.nodes.size() < spectral_config_.min_spectral_nodes){
        spectral_fallback_this_cycle_ = true;
        std::ostringstream stream;
        stream << "compressed support graph has only " << snapshot.nodes.size()
               << " nodes; observing with original EOHDT";
        reason = stream.str();
        spectral_graph_eligible_ = false;
        last_spectral_update_time_ = now;
        UpdateSpectralRuntimeMode(false, false, now);
        return;
    }
    if(spectral_exec_config_.spectral_time_budget_ms > 0.0 &&
       build_ms > spectral_exec_config_.spectral_time_budget_ms){
        spectral_fallback_this_cycle_ = true;
        ++timed_out_spectral_results_;
        ++consecutive_spectral_timeouts_;
        std::ostringstream stream;
        stream << "spectral snapshot exceeded budget: " << build_ms << " ms";
        reason = stream.str();
        if(consecutive_spectral_timeouts_ >= spectral_exec_config_.spectral_timeout_limit){
            EnterRecovery(RecoveryReason::SPECTRAL_TIMEOUTS, now);
        }
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

PartitionEvidence MultiDtgPlus::BuildPartitionEvidence(
    const SpectralResult &result,
    const SpectralGraphSnapshot &snapshot) const{
    PartitionEvidence evidence;
    if(!result.has_valid_cut()) return evidence;
    evidence.ncut = result.diagnostics.ncut;
    evidence.relative_eigengap = RelativeEigengap(
        result.diagnostics.lambda2, result.diagnostics.lambda3,
        spectral_config_.numeric_epsilon);
    evidence.relative_lambda2 = lambda2_ema_initialized_
        ? RelativeLambda2(result.diagnostics.lambda2, lambda2_ema_,
                          spectral_config_.numeric_epsilon)
        : 1.0;
    evidence.cluster0_size = result.diagnostics.cut_left_size;
    evidence.cluster1_size = result.diagnostics.cut_right_size;

    vector<double> cut_clearance;
    vector<double> inside_clearance;
    for(const SpectralEdgeInput &edge : snapshot.edges){
        if(!std::isfinite(edge.clearance) || edge.clearance < 0.0) continue;
        const auto from = result.id_to_index.find(edge.from_h_id);
        const auto to = result.id_to_index.find(edge.to_h_id);
        if(from == result.id_to_index.end() || to == result.id_to_index.end() ||
           from->second >= result.labels.size() || to->second >= result.labels.size()) continue;
        if(result.labels[from->second] != result.labels[to->second]){
            cut_clearance.emplace_back(edge.clearance);
        }
        else{
            inside_clearance.emplace_back(edge.clearance);
        }
    }
    auto median = [](vector<double> values){
        if(values.empty()) return std::numeric_limits<double>::quiet_NaN();
        std::sort(values.begin(), values.end());
        const size_t middle = values.size() / 2U;
        return (values.size() & 1U) != 0U ? values[middle]
            : values[middle - 1U] + 0.5 * (values[middle] - values[middle - 1U]);
    };
    evidence.median_cut_clearance = median(cut_clearance);
    evidence.median_inside_clearance = median(inside_clearance);
    return evidence;
}

void MultiDtgPlus::ConsumeSpectralResult(
    const GlobalRouteContext &context, double now, std::string &reason){
    if(!spectral_job_running_) return;
    const std::future_status status = spectral_future_.wait_for(
        std::chrono::milliseconds(0));
    if(status != std::future_status::ready) return;

    SpectralAsyncOutput output = spectral_future_.get();
    spectral_job_running_ = false;
    if(output.dtg_version != dtg_version_ ||
       output.frontier_version != frontier_version_){
        spectral_fallback_this_cycle_ = true;
        ++stale_spectral_results_;
        partition_confidence_result_ = PartitionConfidenceResult();
        // A stream of stale jobs is weak evidence, not permission to retain an
        // old partition forever.  The normal off-persistence/grace policy
        // handles deactivation while the global TTL blocks use immediately.
        UpdateSpectralRuntimeMode(true, false, now);
        reason = "discarded stale asynchronous spectral result";
        return;
    }
    if(!output.result.success()){
        spectral_fallback_this_cycle_ = true;
        reason = std::string(SpectralSolveStatusName(output.result.status)) +
            ": " + output.result.diagnostics.reason;
        partition_confidence_result_ = PartitionConfidenceResult();
        ++no_cut_epochs_;
        UpdateSpectralRuntimeMode(true, false, now);
        return;
    }
    if(spectral_exec_config_.spectral_time_budget_ms > 0.0 &&
       output.snapshot_build_ms + output.result.diagnostics.total_time_ms >
       spectral_exec_config_.spectral_time_budget_ms){
        spectral_fallback_this_cycle_ = true;
        ++timed_out_spectral_results_;
        ++consecutive_spectral_timeouts_;
        std::ostringstream stream;
        stream << "discarded spectral result over budget: "
               << output.snapshot_build_ms +
                  output.result.diagnostics.total_time_ms << " ms";
        reason = stream.str();
        if(consecutive_spectral_timeouts_ >= spectral_exec_config_.spectral_timeout_limit){
            EnterRecovery(RecoveryReason::SPECTRAL_TIMEOUTS, now);
        }
        return;
    }

    consecutive_spectral_timeouts_ = 0;
    ++spectral_epoch_;
    output.result.diagnostics.total_time_ms += output.snapshot_build_ms;
    last_spectral_result_ = std::move(output.result);
    last_spectral_snapshot_ = std::move(output.snapshot);
    last_raw_spectral_node_count_ = output.raw_node_count;
    last_compressed_spectral_node_count_ = last_spectral_snapshot_.nodes.size();

    if(last_spectral_result_.has_valid_cut()){
        const PartitionEvidence evidence = BuildPartitionEvidence(
            last_spectral_result_, last_spectral_snapshot_);
        partition_confidence_result_ = ComputePartitionConfidence(
            evidence, partition_confidence_config_);
        no_cut_epochs_ = 0;
    }
    else{
        partition_confidence_result_ = PartitionConfidenceResult();
        ++no_cut_epochs_;
        reason = "spectral solve found no useful cut; retained original EOHDT";
    }
    lambda2_ema_ = UpdateLambda2Ema(last_spectral_result_.diagnostics.lambda2,
        lambda2_ema_initialized_ ? lambda2_ema_
                                 : std::numeric_limits<double>::quiet_NaN(),
        spectral_exec_config_.lambda2_ema_alpha);
    lambda2_ema_initialized_ = std::isfinite(lambda2_ema_);

    if(last_spectral_result_.has_valid_cut() &&
       partition_confidence_result_.confidence >=
       spectral_exec_config_.partition_confidence_off){
        stable_spectral_result_ = last_spectral_result_;
        last_spectral_dtg_version_ = output.dtg_version;
        last_spectral_frontier_version_ = output.frontier_version;
        previous_fiedler_ = stable_spectral_result_.fiedler_by_h_id;
        const int previous_active_region = active_region_id_;
        MatchAndUpdatePersistentRegions(stable_spectral_result_, context);
        PublishSpectralStateToHnodes(stable_spectral_result_);
        const bool active_region_lost = previous_active_region >= 0 &&
            regions_.count(previous_active_region) == 0U;
        const bool large_label_change = last_label_change_rate_ >=
            spectral_exec_config_.region_match_threshold;
        if(previous_active_region >= 0 &&
           (active_region_lost || large_label_change ||
            last_partition_change_ == PartitionChangeType::RESET)){
            EnterRecovery(RecoveryReason::PARTITION_CHANGE, now);
        }
    }
    UpdateRegionExecutionState(context);
}

void MultiDtgPlus::UpdateSpectralRuntimeMode(bool eligible,
    bool route_acceptable, double now){
    SpectralModeInput input;
    input.enabled = spectral_exec_config_.enabled;
    input.eligible = eligible;
    input.recovery_requested = recovery_requested_;
    input.recovery_complete = spectral_mode_state_.mode == SpectralMode::RECOVERY &&
        now >= recovery_until_;
    input.route_acceptable = route_acceptable;
    input.partition_confidence = partition_confidence_result_.confidence;
    const SpectralModeUpdate update = DTGPlus::UpdateSpectralMode(
        spectral_mode_state_, input, spectral_mode_config_);
    spectral_mode_state_ = update.state;
    if(update.mode_changed) ++spectral_mode_toggle_count_;
    recovery_requested_ = false;
    partition_valid_ = spectral_mode_state_.mode == SpectralMode::ACTIVE_SOFT;
    if(update.deactivated || spectral_mode_state_.mode == SpectralMode::DISABLED){
        active_region_id_ = -1;
    }
    if(update.exited_recovery){
        recovery_reason_ = RecoveryReason::NONE;
    }
}

void MultiDtgPlus::EnterRecovery(RecoveryReason reason, double now){
    if(spectral_mode_state_.mode != SpectralMode::RECOVERY) ++recovery_count_;
    recovery_reason_ = reason;
    recovery_until_ = std::max(recovery_until_,
        now + spectral_exec_config_.recovery_duration);
    recovery_requested_ = true;
    UpdateSpectralRuntimeMode(true, false, now);
}

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
    const Eigen::Index row = static_cast<Eigen::Index>(from_index->second + 1U);
    const Eigen::Index column = static_cast<Eigen::Index>(to_index->second + 1U);
    if(row < context.distance_matrix.rows() && column < context.distance_matrix.cols()){
        const double exact = context.distance_matrix(row, column);
        if(std::isfinite(exact) && exact < kUnreachableDistance) return exact;
    }
    // The robot-root star is a conservative valid upper bound when a direct
    // pair was deliberately not materialized by the kNN support policy.
    if(from_index->second < context.root_paths.size() &&
       to_index->second < context.root_paths.size()){
        return context.root_paths[from_index->second].first +
               context.root_paths[to_index->second].first;
    }
    return std::numeric_limits<double>::infinity();
}

RouteMetrics MultiDtgPlus::ComputeRouteMetrics(
    const GlobalRouteContext &context, const vector<h_ptr> &route) const{
    RouteMetrics metrics;
    if(route.empty()) return metrics;
    metrics.path_cost = 0.0;
    h_ptr previous;
    int previous_region = active_region_id_;
    std::unordered_set<int> visited_regions;
    if(previous_region >= 0) visited_regions.insert(previous_region);
    for(const h_ptr &h : route){
        const double leg = ContextRouteDistance(context, previous, h);
        if(!std::isfinite(leg) || leg >= kUnreachableDistance){
            metrics.path_cost = std::numeric_limits<double>::infinity();
            return metrics;
        }
        metrics.path_cost += leg;
        const int region = RegionForHnode(h);
        if(region >= 0){
            if(previous_region >= 0 && region != previous_region){
                ++metrics.switch_count;
                if(visited_regions.count(region) != 0U){
                    metrics.revisit_distance += leg;
                }
            }
            visited_regions.insert(region);
            previous_region = region;
        }
        previous = h;
    }
    return metrics;
}

double MultiDtgPlus::EstimateReturnProbability(
    const GlobalRouteContext &context) const{
    if(context.active_hnodes.empty()) return 0.0;
    int reference_region = active_region_id_;
    if(reference_region < 0){
        double closest = std::numeric_limits<double>::infinity();
        for(const h_ptr &h : context.active_hnodes){
            const int region = RegionForHnode(h);
            const double distance = ContextRouteDistance(context, nullptr, h);
            if(region >= 0 && distance < closest){
                closest = distance;
                reference_region = region;
            }
        }
    }
    if(reference_region < 0) return 0.0;
    size_t current_region_frontiers = 0U;
    size_t all_frontiers = 0U;
    std::unordered_set<uint32_t> counted;
    for(const h_ptr &h : context.active_hnodes){
        if(h == nullptr) continue;
        for(const hfe_ptr &edge : h->hf_edges_){
            if(edge == nullptr || edge->tail_n_ == nullptr) continue;
            const uint32_t fid = edge->tail_n_->fid_;
            if(!counted.insert(fid).second) continue;
            if(EROI_ == nullptr || fid >= EROI_->EROI_.size() ||
               static_cast<double>(EROI_->EROI_[fid].unknown_num_) <=
                   spectral_exec_config_.frontier_expected_gain_eps) continue;
            const auto runtime = frontier_runtime_.find(fid);
            if(runtime != frontier_runtime_.end() &&
               (runtime->second.state == FrontierState::QUARANTINED ||
                runtime->second.state == FrontierState::DEAD_FRONTIER)) continue;
            ++all_frontiers;
            const int owner = runtime == frontier_runtime_.end()
                ? RegionForHnode(h) : runtime->second.owner_region;
            if(owner == reference_region) ++current_region_frontiers;
        }
    }
    return all_frontiers == 0U ? 0.0 : Clamp01(
        static_cast<double>(current_region_frontiers) /
        static_cast<double>(all_frontiers));
}

bool MultiDtgPlus::BuildRegionAwareRoute(
    const GlobalRouteContext &context, vector<h_ptr> &route_h,
    double switch_penalty, std::string &reason) const{
    route_h.clear();
    vector<h_ptr> remaining;
    for(const h_ptr &h : context.active_hnodes){
        if(h != nullptr) remaining.emplace_back(h);
    }
    if(remaining.empty()){
        reason = "no effective anchor available for region-aware route";
        return false;
    }

    int reference_region = active_region_id_;
    if(reference_region < 0){
        double closest = std::numeric_limits<double>::infinity();
        for(const h_ptr &h : remaining){
            const int region = RegionForHnode(h);
            const double distance = ContextRouteDistance(context, nullptr, h);
            if(region >= 0 && distance < closest){
                closest = distance;
                reference_region = region;
            }
        }
    }

    h_ptr previous;
    while(!remaining.empty()){
        size_t best = remaining.size();
        double best_score = std::numeric_limits<double>::infinity();

        // The v2 neighbour exception is intentionally evaluated for the first
        // target against robot-to-anchor distance, exactly as proposed.
        if(route_h.empty() && reference_region >= 0){
            size_t best_in = remaining.size();
            size_t best_out = remaining.size();
            double in_distance = std::numeric_limits<double>::infinity();
            double out_distance = std::numeric_limits<double>::infinity();
            for(size_t i = 0; i < remaining.size(); ++i){
                const double distance = ContextRouteDistance(context, nullptr,
                                                             remaining[i]);
                const int region = RegionForHnode(remaining[i]);
                if(region == reference_region && distance < in_distance){
                    in_distance = distance;
                    best_in = i;
                }
                else if(region >= 0 && region != reference_region &&
                        distance < out_distance){
                    out_distance = distance;
                    best_out = i;
                }
            }
            const double in_score = in_distance;
            const double out_score = out_distance + switch_penalty;
            if(best_out != remaining.size() &&
               out_distance <= spectral_exec_config_.neighbor_override_distance &&
               (best_in == remaining.size() ||
                out_score + spectral_exec_config_.neighbor_override_margin <
                in_score)){
                best = best_out;
                best_score = out_score;
            }
        }

        if(best == remaining.size()){
            for(size_t i = 0; i < remaining.size(); ++i){
                const double distance = ContextRouteDistance(context, previous,
                                                             remaining[i]);
                if(!std::isfinite(distance)) continue;
                const int region = RegionForHnode(remaining[i]);
                const double score = distance +
                    ((reference_region >= 0 && region >= 0 &&
                      region != reference_region) ? switch_penalty : 0.0);
                if(score < best_score ||
                   (score == best_score && best != remaining.size() &&
                    remaining[i]->id_ < remaining[best]->id_)){
                    best = i;
                    best_score = score;
                }
            }
        }
        if(best == remaining.size()){
            reason = "region-aware route contains an unreachable anchor";
            return false;
        }
        previous = remaining[best];
        route_h.emplace_back(previous);
        const int selected_region = RegionForHnode(previous);
        if(selected_region >= 0) reference_region = selected_region;
        remaining.erase(remaining.begin() + static_cast<std::ptrdiff_t>(best));
    }
    return true;
}

size_t MultiDtgPlus::EffectiveFrontierCount(double *total_gain) const{
    if(total_gain != nullptr) *total_gain = 0.0;
    if(EROI_ == nullptr) return 0U;
    size_t count = 0U;
    for(size_t fid = 0; fid < EROI_->EROI_.size(); ++fid){
        const auto &frontier = EROI_->EROI_[fid];
        if(frontier.f_state_ != 1U) continue;
        const auto runtime = frontier_runtime_.find(static_cast<uint32_t>(fid));
        if(runtime != frontier_runtime_.end() &&
           (runtime->second.state == FrontierState::QUARANTINED ||
            runtime->second.state == FrontierState::DEAD_FRONTIER)) continue;
        bool alive = false;
        for(size_t vid = 0; vid < frontier.local_vps_.size(); ++vid){
            if(frontier.local_vps_[vid] == 1U){
                alive = true;
                break;
            }
        }
        if(!alive) continue;
        if(static_cast<double>(frontier.unknown_num_) <=
           spectral_exec_config_.frontier_expected_gain_eps) continue;
        ++count;
        if(total_gain != nullptr) *total_gain += std::max(0, frontier.unknown_num_);
    }
    return count;
}

bool MultiDtgPlus::IsLateStage(const GlobalRouteContext &context){
    (void)context;
    double total_gain = 0.0;
    const size_t count = EffectiveFrontierCount(&total_gain);
    if(late_stage_active_){
        if(count >= static_cast<size_t>(
            spectral_exec_config_.late_stage_reactivate_frontier_count)){
            late_stage_active_ = false;
            no_cut_epochs_ = 0;
        }
        return late_stage_active_;
    }
    late_stage_active_ =
        count <= static_cast<size_t>(spectral_exec_config_.late_stage_frontier_count) ||
        total_gain <= spectral_exec_config_.late_stage_total_gain ||
        no_cut_epochs_ >= spectral_exec_config_.late_stage_no_cut_epochs;
    return late_stage_active_;
}

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
               spectral_exec_config_.frontier_expected_gain_eps) continue;
        const auto runtime = frontier_runtime_.find(fid);
        if(runtime == frontier_runtime_.end()) return region_id < 0;
        if(runtime->second.state == FrontierState::QUARANTINED ||
           runtime->second.state == FrontierState::DEAD_FRONTIER) continue;
        if(region_id < 0) return true;
        if(runtime->second.owner_region == region_id &&
           (runtime->second.state == FrontierState::NEW ||
            runtime->second.state == FrontierState::ACTIVE ||
            runtime->second.state == FrontierState::SUSPECT)) return true;
    }
    return false;
}

void MultiDtgPlus::UpdateFrontierRuntimeStates(){
    if(EROI_ == nullptr) return;
    const double now = ros::WallTime::now().toSec();
    for(auto &entry : frontier_runtime_){
        if(entry.first >= EROI_->EROI_.size() ||
           EROI_->EROI_[entry.first].f_state_ != 1U){
            entry.second.state = FrontierState::DEAD_FRONTIER;
        }
    }
    for(size_t index = 0; index < EROI_->EROI_.size(); ++index){
        const auto &frontier = EROI_->EROI_[index];
        if(frontier.f_state_ != 1U) continue;
        const uint32_t fid = static_cast<uint32_t>(index);
        FrontierRuntimeState &runtime = frontier_runtime_[fid];
        const bool first_seen = runtime.last_seen_time <= 0.0;
        runtime.last_seen_time = now;
        runtime.last_seen_frontier_epoch = frontier_epoch_;
        if(runtime.state == FrontierState::DEAD_FRONTIER){
            runtime = FrontierRuntimeState();
            runtime.last_seen_time = now;
            runtime.last_seen_frontier_epoch = frontier_epoch_;
        }
        if(runtime.state == FrontierState::QUARANTINED){
            if(now < runtime.quarantine_until) continue;
            runtime.state = FrontierState::SUSPECT;
            runtime.low_gain_streak = 0;
        }

        bool alive = false;
        for(const uint8_t state : frontier.local_vps_){
            if(state == 1U){ alive = true; break; }
        }
        if(!alive){
            runtime.state = FrontierState::SUSPECT;
            continue;
        }
        if(first_seen || runtime.state == FrontierState::NEW){
            runtime.state = first_seen ? FrontierState::NEW : FrontierState::ACTIVE;
        }

        if(selected_frontier_id_ == fid && runtime.selected_unknown_before >= 0){
            const int actual_gain = std::max(0,
                runtime.selected_unknown_before - frontier.unknown_num_);
            runtime.last_actual_gain = actual_gain;
            if(actual_gain >= spectral_exec_config_.frontier_actual_gain_eps){
                runtime.low_gain_streak = 0;
                runtime.repeat_count = 0;
                runtime.state = FrontierState::ACTIVE;
                runtime.selected_unknown_before = frontier.unknown_num_;
                runtime.last_selected_time = now;
                runtime.last_repeat_count_time = now;
                last_frontier_progress_time_ = now;
                last_region_progress_time_ = now;
            }
            else if(runtime.last_gain_check_frontier_epoch != frontier_epoch_ &&
                    now - runtime.last_selected_time >=
                    spectral_exec_config_.region_stall_window){
                runtime.last_gain_check_frontier_epoch = frontier_epoch_;
                ++runtime.low_gain_streak;
                runtime.state = FrontierState::SUSPECT;
                if(runtime.low_gain_streak >=
                   spectral_exec_config_.frontier_low_gain_cycles){
                    runtime.state = FrontierState::QUARANTINED;
                    runtime.quarantine_until = now +
                        spectral_exec_config_.frontier_quarantine_time;
                }
            }
        }
        if(static_cast<double>(frontier.unknown_num_) <=
           spectral_exec_config_.frontier_expected_gain_eps){
            if(runtime.last_gain_check_frontier_epoch != frontier_epoch_){
                runtime.last_gain_check_frontier_epoch = frontier_epoch_;
                ++runtime.low_gain_streak;
            }
            runtime.state = FrontierState::SUSPECT;
            if(runtime.low_gain_streak >=
               spectral_exec_config_.frontier_low_gain_cycles){
                runtime.state = FrontierState::QUARANTINED;
                runtime.quarantine_until = now +
                    spectral_exec_config_.frontier_quarantine_time;
            }
        }
        runtime.last_unknown_num = frontier.unknown_num_;
    }
}

void MultiDtgPlus::ReassignFrontierOwners(){
    if(EROI_ == nullptr) return;
    std::unordered_map<uint32_t, pair<double, int>> best_owner;
    for(const h_ptr &h : H_list_){
        if(h == nullptr) continue;
        const int region = RegionForHnode(h);
        for(const hfe_ptr &edge : h->hf_edges_){
            if(edge == nullptr || edge->tail_n_ == nullptr ||
               !std::isfinite(edge->length_) || edge->length_ >= kBlockedEdgeDistance) continue;
            const uint32_t fid = edge->tail_n_->fid_;
            const uint8_t vid = edge->tail_n_->vid_;
            if(fid >= EROI_->EROI_.size() || EROI_->EROI_[fid].f_state_ != 1U ||
               vid >= EROI_->EROI_[fid].local_vps_.size() ||
               EROI_->EROI_[fid].local_vps_[vid] != 1U ||
               static_cast<double>(EROI_->EROI_[fid].unknown_num_) <=
                   spectral_exec_config_.frontier_expected_gain_eps) continue;
            const auto existing = best_owner.find(fid);
            const bool candidate_is_labeled = region >= 0;
            const bool existing_is_labeled = existing != best_owner.end() &&
                existing->second.second >= 0;
            if(existing == best_owner.end() ||
               (candidate_is_labeled && !existing_is_labeled) ||
               (candidate_is_labeled == existing_is_labeled &&
                edge->length_ < existing->second.first)){
                best_owner[fid] = {edge->length_, region};
            }
        }
    }
    for(auto &entry : frontier_runtime_){
        FrontierRuntimeState &runtime = entry.second;
        if(runtime.state == FrontierState::DEAD_FRONTIER ||
           runtime.state == FrontierState::QUARANTINED) continue;
        const auto best = best_owner.find(entry.first);
        const int owner = best == best_owner.end() ? -1 : best->second.second;
        if(owner != runtime.owner_region){
            runtime.owner_region = owner;
            ++frontier_reassignment_count_;
        }
        if(owner >= 0 && active_region_id_ >= 0 && owner != active_region_id_){
            runtime.state = FrontierState::DEFERRED;
        }
        else if(runtime.state == FrontierState::DEFERRED){
            runtime.state = FrontierState::ACTIVE;
        }
    }
}

void MultiDtgPlus::RecordSelectedFrontier(uint32_t frontier_id){
    if(EROI_ == nullptr || frontier_id >= EROI_->EROI_.size()) return;
    const double now = ros::WallTime::now().toSec();
    FrontierRuntimeState &runtime = frontier_runtime_[frontier_id];
    if(selected_frontier_id_ == frontier_id &&
       runtime.last_repeat_count_time >= 0.0 &&
       now - runtime.last_repeat_count_time >= 0.5){
        ++runtime.repeat_count;
        ++repeated_target_count_;
        runtime.last_repeat_count_time = now;
    }
    else if(selected_frontier_id_ != frontier_id){
        runtime.repeat_count = 1;
        runtime.selected_unknown_before = EROI_->EROI_[frontier_id].unknown_num_;
        runtime.last_selected_time = now;
        runtime.last_repeat_count_time = now;
    }
    selected_frontier_id_ = frontier_id;
    if(runtime.repeat_count >= spectral_exec_config_.repeat_target_limit){
        runtime.state = FrontierState::QUARANTINED;
        runtime.quarantine_until = now +
            spectral_exec_config_.frontier_quarantine_time;
        EnterRecovery(RecoveryReason::REPEATED_TARGET, now);
    }
}

bool MultiDtgPlus::DetectAndHandleRegionStall(double now){
    if(active_region_id_ < 0) return false;
    size_t blocking = 0U;
    double unknown = 0.0;
    for(const auto &entry : frontier_runtime_){
        if(entry.second.owner_region != active_region_id_ ||
           (entry.second.state != FrontierState::NEW &&
            entry.second.state != FrontierState::ACTIVE &&
            entry.second.state != FrontierState::SUSPECT)) continue;
        ++blocking;
        if(EROI_ != nullptr && entry.first < EROI_->EROI_.size()){
            unknown += std::max(0, EROI_->EROI_[entry.first].unknown_num_);
        }
    }
    if(watchdog_region_id_ != active_region_id_){
        watchdog_region_id_ = active_region_id_;
        watchdog_blocking_frontiers_ = blocking;
        watchdog_unknown_gain_ = unknown;
        last_region_progress_time_ = now;
        return false;
    }
    if(blocking < watchdog_blocking_frontiers_ ||
       unknown + spectral_exec_config_.frontier_actual_gain_eps <
       watchdog_unknown_gain_){
        last_region_progress_time_ = now;
    }
    watchdog_blocking_frontiers_ = blocking;
    watchdog_unknown_gain_ = unknown;
    if(blocking == 0U){
        last_region_progress_time_ = now;
        return false;
    }
    if(now - last_region_progress_time_ >
       spectral_exec_config_.region_stall_timeout){
        EnterRecovery(RecoveryReason::REGION_STALL, now);
        last_region_progress_time_ = now;
        return true;
    }
    return false;
}

void MultiDtgPlus::RecordActiveRegionPathResult(bool success){
    if(active_region_id_ < 0) return;
    auto active = regions_.find(active_region_id_);
    if(active == regions_.end()){
        active_region_id_ = -1;
        return;
    }
    if(success){
        active->second.unreachable_streak = 0;
        return;
    }
    if(active->second.last_unreachable_frontier_epoch == frontier_epoch_) return;
    active->second.last_unreachable_frontier_epoch = frontier_epoch_;
    ++active->second.unreachable_streak;
    if(active->second.unreachable_streak <
       spectral_exec_config_.unreachable_confirm_cycles) return;

    const int unreachable_region = active_region_id_;
    active->second.exec_state = RegionExecState::UNREACHABLE;
    active->second.unreachable_spectral_epoch = spectral_epoch_;
    active_region_id_ = -1;
    missed_region_set_.erase(unreachable_region);
    std::deque<int> remaining;
    for(const int id : missed_regions_){
        if(id != unreachable_region) remaining.push_back(id);
    }
    missed_regions_.swap(remaining);
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

double MultiDtgPlus::SpectralGainMultiplier(const h_ptr &candidate_h,
    double travel_distance) const{
    if(!soft_route_selected_ || !spectral_exec_config_.enabled || candidate_h == nullptr ||
       spectral_mode_state_.mode != SpectralMode::ACTIVE_SOFT ||
       !stable_spectral_result_.has_valid_cut() ||
       !std::isfinite(stable_spectral_result_.diagnostics.cut_threshold)){
        return 1.0;
    }
    const auto candidate_fiedler =
        stable_spectral_result_.fiedler_by_h_id.find(candidate_h->id_);
    if(candidate_fiedler == stable_spectral_result_.fiedler_by_h_id.end()){
        return 1.0;
    }

    double maximum_cut_distance = 0.0;
    for(Eigen::Index i = 0; i < stable_spectral_result_.fiedler.size(); ++i){
        maximum_cut_distance = std::max(maximum_cut_distance,
            std::abs(stable_spectral_result_.fiedler(i) -
                     stable_spectral_result_.diagnostics.cut_threshold));
    }
    const double normalized_distance = std::abs(candidate_fiedler->second -
        stable_spectral_result_.diagnostics.cut_threshold) /
        std::max(maximum_cut_distance, spectral_config_.numeric_epsilon);
    const double boundary_penalty = std::max(0.0,
        std::min(1.0, 1.0 - normalized_distance));
    const int candidate_region = RegionForHnode(candidate_h);
    const double cross_penalty = partition_valid_ && active_region_id_ >= 0 &&
        candidate_region >= 0 && candidate_region != active_region_id_ ? 1.0 : 0.0;

    size_t current_frontiers = 0U;
    size_t all_frontiers = 0U;
    for(const auto &entry : frontier_runtime_){
        if(entry.second.state == FrontierState::QUARANTINED ||
           entry.second.state == FrontierState::DEAD_FRONTIER) continue;
        ++all_frontiers;
        if(entry.second.owner_region == active_region_id_) ++current_frontiers;
    }
    const double return_probability = all_frontiers == 0U ? 0.0 :
        static_cast<double>(current_frontiers) / static_cast<double>(all_frontiers);
    double switch_cost = ComputeDynamicSwitchPenalty(
        spectral_exec_config_.switch_penalty_base,
        partition_confidence_result_.confidence, return_probability) * cross_penalty;
    // Local target scoring cannot see the best in-region competitor.  It uses
    // the conservative distance half of the neighbour exception; the exact
    // distance+margin comparison is enforced by BuildRegionAwareRoute.
    if(cross_penalty > 0.0 && travel_distance <=
       spectral_exec_config_.neighbor_override_distance){
        switch_cost = 0.0;
    }
    const double extra_distance = switch_cost +
        spectral_exec_config_.spectral_view_weight * boundary_penalty *
        std::max(0.0, travel_distance);
    const double extra_time = extra_distance /
        std::max(v_max_, spectral_config_.numeric_epsilon);
    return std::exp(-lambda_e_ * extra_time);
}

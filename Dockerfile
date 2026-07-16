FROM osrf/ros:noetic-desktop-full

SHELL ["/bin/bash", "-c"]

# ── Replace apt sources with Tsinghua mirrors (Ubuntu only, keep ROS repo as-is) ──
RUN sed -i 's|http://archive.ubuntu.com/ubuntu/|https://mirrors.tuna.tsinghua.edu.cn/ubuntu/|g' /etc/apt/sources.list && \
    sed -i 's|http://security.ubuntu.com/ubuntu/|https://mirrors.tuna.tsinghua.edu.cn/ubuntu/|g' /etc/apt/sources.list

# ── Install CUDA 11.8 toolkit (compiler + dev libs only, skip Nsight/profilers) ──
RUN apt-get update && apt-get install -y --no-install-recommends wget gnupg && \
    wget -q https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/cuda-keyring_1.0-1_all.deb && \
    dpkg -i cuda-keyring_1.0-1_all.deb && rm cuda-keyring_1.0-1_all.deb && \
    apt-get update && apt-get install -y --no-install-recommends \
    cuda-compiler-11-8 \
    cuda-cudart-dev-11-8 \
    cuda-libraries-dev-11-8 \
    && ln -s /usr/local/cuda-11.8 /usr/local/cuda && \
    echo 'export PATH=/usr/local/cuda/bin:$PATH' >> /etc/bash.bashrc && \
    echo 'export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH' >> /etc/bash.bashrc

# ── Install system deps + configure git mirror for GitHub (China CDN) ──
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    ca-certificates \
    git \
    cmake \
    build-essential \
    protobuf-compiler \
    libgoogle-glog-dev \
    libgflags-dev \
    libzmqpp-dev \
    libdw-dev \
    libdwarf-dev \
    libarmadillo-dev \
    libeigen3-dev \
    libpcl-dev \
    python3-pip \
    python3-osrf-pycommon \
    && rm -rf /var/lib/apt/lists/* \
    && git config --global url."https://gitclone.com/github.com/".insteadOf "https://github.com/" \
    && git config --global http.postBuffer 524288000

# ── Install ROS dependencies for EDEN ──
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    ros-noetic-joy \
    ros-noetic-octomap-ros \
    ros-noetic-control-toolbox \
    ros-noetic-mavros \
    ros-noetic-mavros-extras \
    ros-noetic-pcl-ros \
    ros-noetic-pcl-conversions \
    ros-noetic-cv-bridge \
    ros-noetic-image-transport \
    ros-noetic-gazebo-ros-pkgs \
    ros-noetic-gazebo-ros-control \
    ros-noetic-ros-control \
    ros-noetic-ros-controllers \
    ros-noetic-rqt \
    ros-noetic-rqt-common-plugins \
    ros-noetic-nodelet \
    ros-noetic-dynamic-reconfigure \
    ros-noetic-cmake-modules \
    ros-noetic-octomap-msgs \
    ros-noetic-geographic-msgs \
    && rm -rf /var/lib/apt/lists/*

# ── Install graphics libs + Open3D runtime deps (filament needs libc++) ──
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    libgl1-mesa-dev \
    libglfw3-dev \
    libgles2-mesa-dev \
    libglu1-mesa-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libc++1 \
    libc++abi1 \
    liblzf-dev \
    && rm -rf /var/lib/apt/lists/*

# ── Install gflags_catkin and glog_catkin from source ──
COPY catkin_deps_src /tmp/catkin_deps/src
RUN source /opt/ros/noetic/setup.bash && \
    cd /tmp/catkin_deps && \
    catkin_make install -DCMAKE_INSTALL_PREFIX=/opt/ros/noetic -DUSE_SYSTEM_GFLAGS=ON -DUSE_SYSTEM_GLOG=ON && \
    rm -rf /tmp/catkin_deps

# ── Set up catkin workspace skeleton ──
RUN mkdir -p /root/catkin_ws/src

# ── Entrypoint ──
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]

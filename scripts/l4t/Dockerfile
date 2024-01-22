FROM arm64v8/ubuntu:18.04
RUN apt-get update && apt-get install -y gnupg wget \
  && apt-key adv --fetch-key "https://newrepo.switchroot.org/pubkey" \
  && echo > /etc/apt/sources.list.d/99-switchroot-newrepo.list "deb https://newrepo.switchroot.org/ switchroot unstable" \
  && apt-key adv --fetch-key "https://apt.kitware.com/keys/kitware-archive-latest.asc" \
  && echo > /etc/apt/sources.list.d/kitware.list "deb https://apt.kitware.com/ubuntu/ bionic main" \
  && apt-key adv --fetch-key "https://repo.download.nvidia.com/jetson/jetson-ota-public.asc" \
  && echo >> /etc/apt/sources.list.d/nvidia-l4t-apt-source.list "deb https://repo.download.nvidia.com/jetson/common r32.6 main" \
  && echo >> /etc/apt/sources.list.d/nvidia-l4t-apt-source.list "deb https://repo.download.nvidia.com/jetson/t210 r32.6 main" \
  && mkdir -p /opt/nvidia/l4t-packages && touch /opt/nvidia/l4t-packages/.nv-l4t-disable-boot-fw-update-in-preinstall /etc/nv_boot_control.conf \
  && apt-get update && apt-get install -y --no-install-recommends g++-8 nasm cmake sudo git gcc- manpages- \
  && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 20 \
  && update-alternatives --install /usr/bin/cpp cpp /usr/bin/g++-8 20 \
  && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 20 \
  && update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-8 20 \
  && apt-get remove -y gnupg && apt-get autoremove -y \
  && rm -rf /var/lib/apt/lists/* /usr/share/man/*

RUN apt-get update && apt-get install -y --no-install-recommends xorg-dev libdbus-1-dev libssl-dev \
    libass-dev libwebp-dev libv4l-dev libpulse-dev libuchardet-dev nvidia-l4t-jetson-multimedia-api \
  && rm -rf /var/lib/apt/lists/* /usr/share/man/* /usr/src/tensorrt/*

RUN adduser --gecos '' --disabled-password l4t \
  && echo 'l4t ALL=(ALL) NOPASSWD:ALL' > /etc/sudoers.d/l4t

USER l4t
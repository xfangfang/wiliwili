# docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
# docker build --network host -t build:switchroot-bionic .
# docker run --rm --name switchroot --network host -v $PWD:/opt -w /opt -it build:switchroot-bionic bash -l

FROM arm64v8/ubuntu:18.04
RUN apt-get update && apt-get install -y gnupg wget \
  && wget -qO- https://newrepo.switchroot.org/pubkey | gpg --dearmor -o /etc/apt/trusted.gpg.d/switchroot.gpg \
  && echo > /etc/apt/sources.list.d/99-switchroot-newrepo.list "deb https://newrepo.switchroot.org/ switchroot unstable" \
  && wget -qO- https://apt.kitware.com/keys/kitware-archive-latest.asc | gpg --dearmor -o /etc/apt/trusted.gpg.d/kitware.gpg \
  && echo > /etc/apt/sources.list.d/kitware.list "deb https://apt.kitware.com/ubuntu/ bionic main" \
  && wget -qO- https://repo.download.nvidia.com/jetson/jetson-ota-public.asc | gpg --dearmor -o /etc/apt/trusted.gpg.d/jetson.gpg \
  && echo >> /etc/apt/sources.list.d/nvidia-l4t-apt-source.list "deb https://repo.download.nvidia.com/jetson/common r32.6 main" \
  && echo >> /etc/apt/sources.list.d/nvidia-l4t-apt-source.list "deb https://repo.download.nvidia.com/jetson/t210 r32.6 main" \
  && mkdir -p /opt/nvidia/l4t-packages && touch /opt/nvidia/l4t-packages/.nv-l4t-disable-boot-fw-update-in-preinstall /etc/nv_boot_control.conf \
  && apt-get update && apt-get install -y --no-install-recommends g++-8 nasm cmake sudo git gcc- manpages- \
  && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60 --slave /usr/bin/cc cc /usr/bin/gcc-8 \
  && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 60 --slave /usr/bin/cpp cpp /usr/bin/g++-8 \
  && apt-get remove -y gnupg && apt-get autoremove -y && rm -rf /var/lib/apt/lists/* /usr/share/man/*

RUN apt-get update && apt-get install -y --no-install-recommends xorg-dev libdbus-1-dev libssl-dev \
    libass-dev libwebp-dev libv4l-dev libpulse-dev libuchardet-dev nvidia-l4t-jetson-multimedia-api \
  && rm -rf /var/lib/apt/lists/* /usr/share/man/* /usr/src/tensorrt/*

RUN adduser --gecos '' --disabled-password ubuntu \
  && echo 'ubuntu ALL=(ALL) NOPASSWD:ALL' > /etc/sudoers.d/ubuntu

USER ubuntu
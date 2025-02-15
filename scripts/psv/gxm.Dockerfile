FROM vitasdk/vitasdk:latest

MAINTAINER xfangfang <xfangfang@126.com>

RUN apk update && \
    apk add cmake ninja meson pkgconf bash git zstd tar patch && \
    git config --global --add safe.directory $(pwd)

# Install VDPM Dependencies
ADD . /vdpm
RUN vdpm libass harfbuzz fribidi freetype libpng libwebp && \
    adduser --gecos '' --disabled-password builder && \
    echo 'builder ALL=(ALL) NOPASSWD:ALL' > /etc/sudoers.d/builder && \
    chown -R builder:builder /vdpm && \
    ls -l /vdpm && \
    su - builder -c "cd /vdpm/mbedtls && vita-makepkg" && \
    vdpm /vdpm/mbedtls/*-arm.tar.xz && \
    touch /tmp/vdpm_install_mbedtls && \
    su - builder -c "cd /vdpm/ffmpeg && vita-makepkg" && \
    vdpm /vdpm/ffmpeg/*-arm.tar.xz && \
    touch /tmp/vdpm_install_ffmpeg && \
    su - builder -c "cd /vdpm/mpv_gxm && vita-makepkg" && \
    vdpm /vdpm/mpv_gxm/*-arm.tar.xz && \
    touch /tmp/vdpm_install_mpv && \
    su - builder -c "cd /vdpm/curl && vita-makepkg" && \
    vdpm /vdpm/curl/*-arm.tar.xz && \
    touch /tmp/vdpm_install_curl && \
    rm -rf /vdpm

RUN mkdir /src/ &&\
    echo \#\!/bin/bash -i >> /entrypoint.sh &&\
    echo >> /entrypoint.sh &&\
    echo "set -e" >> /entrypoint.sh &&\
    echo "cd /src" >> /entrypoint.sh &&\
    echo "echo \"\$@\"" >> /entrypoint.sh &&\
    echo "bash -c \"\$@\"" >> /entrypoint.sh &&\
    chmod +x /entrypoint.sh

VOLUME /src/
WORKDIR /src/
SHELL ["/bin/bash", "-i", "-c"]
ENTRYPOINT ["/entrypoint.sh"]

FROM vitasdk/vitasdk:latest

MAINTAINER xfangfang <xfangfang@126.com>

RUN apk update && \
    apk add cmake ninja meson pkgconf bash git zstd tar patch && \
    git config --global --add safe.directory $(pwd)

# Copy PVR_PSP2 (GLES) to vita toolchain dir
RUN mkdir -p /vita/dependencies/include && \
    mkdir -p /vita/dependencies/lib && \
    mkdir -p /vita/dependencies/suprx && \
    pvr_psp2_version=3.9 && \
    wget https://github.com/GrapheneCt/PVR_PSP2/archive/refs/tags/v$pvr_psp2_version.zip -P/tmp && \
    unzip /tmp/v$pvr_psp2_version.zip -d/tmp && \
    cp -r /tmp/PVR_PSP2-$pvr_psp2_version/include/* /vita/dependencies/include && \
    sed -i -e s/__drvkhrplatform_h_/__khrplatform_h_/ /vita/dependencies/include/KHR/khrplatform.h && \
    wget https://github.com/GrapheneCt/PVR_PSP2/releases/download/v$pvr_psp2_version/vitasdk_stubs.zip -P/tmp && \
    unzip /tmp/vitasdk_stubs.zip -d/tmp/pvr_psp2_stubs && \
    find /tmp/pvr_psp2_stubs -type f -name "*.a" -exec cp {} /vita/dependencies/lib \; && \
    wget https://github.com/GrapheneCt/PVR_PSP2/releases/download/v$pvr_psp2_version/PSVita_Release.zip -P/tmp && \
    unzip /tmp/PSVita_Release.zip -d/tmp/PSVita_Release && \
    rm /tmp/PSVita_Release/libGLESv1_CM.suprx && \
    rm /tmp/PSVita_Release/libpvr2d.suprx && \
    mv /tmp/PSVita_Release/*.suprx /vita/dependencies/suprx/ && \
    cp -rv /vita/dependencies/* ${VITASDK}/arm-vita-eabi && \
    rm -rf /vita && \
    rm -rf /tmp/*

# Install VDPM Dependencies
ADD . /vdpm
RUN vdpm mbedtls libass harfbuzz fribidi freetype libpng libwebp && \
    adduser --gecos '' --disabled-password builder && \
    echo 'builder ALL=(ALL) NOPASSWD:ALL' > /etc/sudoers.d/builder && \
    chown -R builder:builder /vdpm && \
    ls -l /vdpm && \
    su - builder -c "cd /vdpm/ffmpeg && vita-makepkg" && \
    su - builder -c "cd /vdpm/curl && vita-makepkg" && \
    su - builder -c "cd /vdpm/sdl2 && vita-makepkg" && \
    vdpm /vdpm/ffmpeg/*-arm.tar.xz && \
    vdpm /vdpm/sdl2/*-arm.tar.xz && \
    touch /tmp/vdpm_install_ffmpeg && \
    touch /tmp/vdpm_install_sdl2 && \
    su - builder -c "cd /vdpm/mpv && vita-makepkg" && \
    vdpm /vdpm/curl/*-arm.tar.xz && \
    vdpm /vdpm/mpv/*-arm.tar.xz && \
    touch /tmp/vdpm_install_curl && \
    touch /tmp/vdpm_install_mpv && \
    rm -rf /vdpm

RUN mkdir /src/ &&\
    echo \#\!/bin/bash -i >> /entrypoint.sh &&\
    echo >> /entrypoint.sh &&\
    echo "set -e" >> /entrypoint.sh &&\
    echo "mkdir -p /src/scripts/psv/module/" >> /entrypoint.sh &&\
    echo "cp ${VITASDK}/arm-vita-eabi/suprx/*.suprx /src/scripts/psv/module/" >> /entrypoint.sh &&\
    echo "cd /src" >> /entrypoint.sh &&\
    echo "echo \"\$@\"" >> /entrypoint.sh &&\
    echo "bash -c \"\$@\"" >> /entrypoint.sh &&\
    chmod +x /entrypoint.sh

VOLUME /src/
WORKDIR /src/
SHELL ["/bin/bash", "-i", "-c"]
ENTRYPOINT ["/entrypoint.sh"]

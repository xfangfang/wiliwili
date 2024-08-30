FROM xfangfang/pacbrew:latest

MAINTAINER xfangfang <xfangfang@126.com>

RUN BASE_URL="https://github.com/xfangfang/wiliwili/releases/download/v0.1.0" && \
    pacbrew-pacman -Syyu --noconfirm && \
    pacbrew-pacman -U --noconfirm $BASE_URL/ps4-openorbis-sdl2-2.0.18-18-any.pkg.tar.xz && \
    pacbrew-pacman -U --noconfirm $BASE_URL/ps4-openorbis-musl-1.5-2-any.pkg.tar.xz && \
    pacbrew-pacman -U --noconfirm $BASE_URL/ps4-openorbis-mbedtls-2.28.5-1-any.pkg.tar.xz  && \
    pacbrew-pacman -U --noconfirm $BASE_URL/ps4-openorbis-libcurl-7.80.0-3-any.pkg.tar.xz && \
    pacbrew-pacman -U --noconfirm $BASE_URL/ps4-openorbis-cpr-1.10.5-1-any.pkg.tar.xz && \
    pacbrew-pacman -U --noconfirm $BASE_URL/ps4-openorbis-ffmpeg-6.0-2-any.pkg.tar.xz && \
    pacbrew-pacman -U --noconfirm $BASE_URL/ps4-openorbis-libmpv-0.36.0-1-any.pkg.tar.xz && \
    pacbrew-pacman -S --needed --noconfirm ps4-openorbis-libwebp && \
    yes | pacbrew-pacman -Scc

ENV PATH="/opt/pacbrew/ps4/openorbis/bin:/opt/pacbrew/ps4/openorbis/usr/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
ENV OPENORBIS="/opt/pacbrew/ps4/openorbis"
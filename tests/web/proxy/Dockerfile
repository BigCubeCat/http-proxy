FROM archlinux:latest

RUN pacman -Sy gtest spdlog --noconfirm

COPY http_proxy .

RUN ls

CMD ["./http_proxy", "--port", "9000"]

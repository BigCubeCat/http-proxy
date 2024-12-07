FROM archlinux
RUN pacman --noconfirm -Sy spdlog cmake make clang gtest
COPY . /bin
WORKDIR /bin/
RUN cmake -S . -B build && cmake --build build
CMD ["./build/http_proxy"]


FROM --platform=linux/amd64 ubuntu:22.04 as builder

RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential clang

COPY . /repo
WORKDIR /repo
RUN chmod +x Build.sh
RUN sed -i '/rm/d' Build.sh
RUN ./Build.sh
RUN rm /repo/Main.o
RUN clang++ /repo/mayhem/fuzz_printIndentToDepth.cpp /repo/*.o -fsanitize=fuzzer,address -I /repo/src/ -o /fuzz

FROM ubuntu:22.04 as package
COPY --from=builder /repo/bin/cakelisp /
COPY --from=builder /fuzz /fuzz_printIndentToDepth

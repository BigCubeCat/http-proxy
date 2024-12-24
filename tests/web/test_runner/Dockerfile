FROM alpine:latest
WORKDIR /app
COPY test_script.sh .
RUN chmod +x test_script.sh
RUN apk add curl
CMD ["/app/test_script.sh"]


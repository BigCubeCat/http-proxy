#!/bin/bash

cd ../tests/web/

echo "test file"
curl --proxy1.0 http://localhost:9000 -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8081/largefile.zip | grep -q "200" || exit 1

echo "test site"
curl --proxy1.0 http://localhost:9000 -s http://127.0.0.1:8082/ | grep -q "Welcome to the text-only site!" || exit 1

echo "test redirect"
curl --proxy1.0 http://localhost:9000 -s -o /dev/null -w "%{http_code}" -L http://127.0.0.1:8083/ | grep -q "200" || exit 1



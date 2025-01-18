cd ../tests/os-proxy-tests/
./2-concurrent-batches.sh --port 9000 --url http://xcal1.vodafone.co.uk/5MB.zip --requests 50 --timeout 20 --concurrency 20
# ./2-concurrent-batches.sh --port 9000 --url http://localhost:8081 --requests 50 --timeout 20 --concurrency 20
# ./2-concurrent-batches.sh --port 9000 --url http://xcal1.vodafone.co.uk/5MB.zip --requests 50 --timeout 10 --concurrency 20

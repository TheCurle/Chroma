echo "Build"
make kernel

echo "POST:"
./post.sh

echo "RUN"
./run.bat
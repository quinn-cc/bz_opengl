# Download vcpkg
git clone https://github.com/Microsoft/vcpkg.git

# Bootstrap vcpkg
cd ./vcpkg
./bootstrap-vcpkg.sh

# Install dependencies
./vcpkg install

# Return to project root
cd ..
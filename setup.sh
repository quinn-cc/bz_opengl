# Download vcpkg
git clone https://github.com/Microsoft/vcpkg.git

# Bootstrap vcpkg
./vcpkg/bootstrap-vcpkg.sh

# Install dependencies
./vcpkg/vcpkg install

# Return to project root
cd ..
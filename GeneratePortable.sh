echo "creating a portable version"

mkdir -p "portable"
cp -v bin/release/Engine.exe portable/
cp -v externals/amd_ags_x64.dll portable/
cp -v externals/assimp.dll portable/
cp -v runExperiment.sh portable/

cp -v -r script portable/
mkdir -p "portable/src"
cp -v -r src/shaders portable/src/
cp -v -r assets portable/
cp -v -r big_assets portable/
cp -v -r experiment portable/


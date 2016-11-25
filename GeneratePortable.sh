echo "creating a portable version"

mkdir -p "portable"
cp bin/release/Engine.exe portable/
cp externals/amd_ags_x64.dll portable/
cp externals/assimp.dll portable/
cp runExperiment.sh portable/

cp -r script portable/
mkdir -p "portable/src"
cp -r src/shaders portable/src/
cp -r assets portable/
cp -r big_assets portable/
cp -r experiment portable/


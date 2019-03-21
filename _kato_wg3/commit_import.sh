rm -rf ../framework/base
rm -rf ../framework/interfaces
rm -rf ../framework/gfxdevices
cp -rf imports/* ../framework
ruby flip_files.rb ../framework/base/* ../framework/interfaces/* ../framework/gfxdevices/**/*

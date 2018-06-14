#! /bin/sh

git pull
cp mnist/readme.md /tmp/
rm -fr mnist/
cp -r /data/users/rockyliu/fbsource/fbcode/experimental/rockyliu/mnist/ .
cp /data/users/rockyliu/notebooks/MNIST.ipynb mnist/
cp /tmp/readme.md mnist/
git add .
git commit
git push

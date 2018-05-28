#! /bin/sh

rm -fr mnist/
cp -r /data/users/rockyliu/fbsource/fbcode/experimental/rockyliu/mnist/ .
cp /data/users/rockyliu/notebooks/MNIST.ipynb mnist/
git add .
git commit
git push

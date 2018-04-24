#! /bin/sh

rm -fr mnist/
cp -r /data/users/rockyliu/fbsource/fbcode/experimental/rockyliu/mnist/ .
git add .
git commit
git push

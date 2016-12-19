library(ISLR)

pca = function() {
  print(apply(USArrests, 2, mean))
  print(apply(USArrests, 2, var))
  print(apply(USArrests, 2, sd))
  
  pr.out = prcomp(USArrests, scale = T)
  print(pr.out$rotation)
  
  # Not sure what's the meaning of scale; from the plot it does not 
  # seem it uses the exactly the loading vectors.
  biplot(pr.out, scale = 0)
  
  pr.var = pr.out$sdev^2
  pve = pr.var / sum(pr.var)
  print(pve)
  
  pve2 = c(var(pr.out$x[,1]), var(pr.out$x[,2]), var(pr.out$x[,3]), var(pr.out$x[,4]))
  pve2 = pve2 / sum(pve2)
  print(pve2)
  
  m = ncol(USArrests)
  X = USArrests
  for (i in 1:m) {
    X[,i] = X[,i] - pr.out$center[i]
    X[,i] = X[,i] / pr.out$scale[i]
  }
  
  v = var(X)
  total = v[1,1] + v[2,2] + v[3,3] + v[4,4]
  # The two ways of obtaining the total variance gives the same reuslt, 
  # which is expected
  print(sprintf("Sum of variance: %f; total variance: %f", sum(pr.var), total))
  
  par(mfrow=c(1,2))
  plot(pve, type = 'b')
  plot(cumsum(pve), type = 'b')
}

k.means = function() {
  par(mfrow = c(1,2))
  
  set.seed(2)
  X = matrix(rnorm(50 * 2), ncol = 2)
  X[1:25, 1] = X[1:25, 1] + 3
  X[1:25, 2] = X[1:25, 2] - 4
  
  km.out = kmeans(X, 2, nstart = 20)
  print(km.out$cluster)
  plot(X, col = km.out$cluster, pch = 20)
  
  km.out.3 = kmeans(X, 3, nstart = 20)
  plot(X, col = km.out.3$cluster, pch = 20)
  
  set.seed(3)
  print(sprintf("%f %f", 
                kmeans(X, 3, nstart = 20)$tot.withinss, 
                kmeans(X, 3, nstart = 1)$tot.withinss))
}

hier = function() {
  set.seed(2)
  X = matrix(rnorm(50 * 2), ncol = 2)
  X[1:25, 1] = X[1:25, 1] + 3
  X[1:25, 2] = X[1:25, 2] - 4
  # d = dist(X)
  # return(d)
  
  hc.complete = hclust(dist(X), method = "complete")
  hc.average = hclust(dist(X), method = "average")
  hc.single = hclust(dist(X), method = "single")
  hc.centroid = hclust(dist(X), method = "centroid")
  
  hc.complete.scaled = hclust(dist(scale(X)), method = "complete")
  
  par(mfrow = c(1,2))
  plot(hc.complete, main = "complete", cex = .9, xlab = "", sub = "")
  plot(hc.complete.scaled, main = "complete.scaled", cex = .9, xlab = "", sub = "")
  # plot(hc.average, main = "average", cex = .9, xlab = "", sub = "")
  # plot(hc.single, main = "single", cex = .9, xlab = "", sub = "")
  # plot(hc.centroid, main = "centroid", cex = .9, xlab = "", sub = "")
  
  print(cutree(hc.complete, 2))
  print(cutree(hc.average, 2))
  print(cutree(hc.single, 2))
  
  par(mfrow = c(1,1))
  X = matrix(rnorm(30 * 3), ncol = 3)
  
  # Why do we need to transpose the matrix: 
  # because we need to calculate the correlation between instances based on features
  dd = as.dist(1 - cor(t(X)))
  
  plot(hclust(dd, method = "complete"), main = "complete", cex = .9, xlab = "", sub = "")
}

nci60 = function() {
  data = NCI60$data
  labs = NCI60$labs
  
  Cols = function(vec) {
    cols = rainbow(length(unique(vec)))
    return(cols[as.numeric(as.factor(vec))])
  }
  
  pr.out = prcomp(data, scale = T)
  par(mfrow = c(1,2))
  plot(pr.out$x[,1:2], col = Cols(labs), pch = 19, xlab = "Z1", ylab = "Z2")
  plot(pr.out$x[,c(1,3)], col = Cols(labs), pch = 19, xlab = "Z1", ylab = "Z3")
  
  print(summary(pr.out))
  
  # Plot pr.out$sdev^2
  # plot(pr.out)
  
  pve = 100 * (pr.out$sdev ^ 2) / sum(pr.out$sdev ^ 2)
  plot(pve, type = 'o', col = "blue")
  plot(cumsum(pve), type = 'o', col = "brown3")
  
  par(mfrow = c(1,1))
  data.scaled = scale(data)
  data.dist = dist(data.scaled)
  hc = hclust(data.dist, method = "complete")
  hcluster.ret = cutree(hc, 4)
  print(table(hcluster.ret, labs))
  
  plot(hc, labels = labs, xlab = "", ylab = "", sub = "")
  abline(h = 139, col = "red")
  
  kmeans.out = kmeans(data.scaled, 4, nstart = 20)
  print(table(kmeans.out$cluster, hcluster.ret))
  
  hc = hclust(dist(pr.out$x[,1:5]), method = "complete")
  plot(hc, labels = labs, xlab = "", ylab = "", sub = "")
  hc.ret = cutree(hc, 4)
  print(table(hc.ret, labs))
}

ex2 = function() {
  dist = matrix(
          c(0, 0.3, 0.4, 0.7,
            0.3, 0, 0.5, 0.8,
            0.4, 0.5, 0, 0.45,
            0.7, 0.8, 0.45, 0),
          ncol = 4)
  d = as.dist(dist)
  print(d)
  hc = hclust(d, method = "centroid")
  plot(hc, labels = c(1,2,3,4))
}

ex7 = function() {
  scaled = scale(USArrests)
  d1 = dist(scaled)^2
  d2 = as.dist(1 - cor(t(scaled)))
  
  # plot(d1, d2)
  r = d1 / d2
  hist(r, xlim = c(0, 40), breaks = 100000)
  
  summary(r)
}

ex9 = function() {
  par(mfrow = c(1,1))
  scaled = scale(USArrests)
  hc = hclust(dist(scaled), method = "complete")
  plot(hc, xlab = "", ylab = "", sub = "")
  
  # Not scaled version
  hc = hclust(dist(USArrests), method = "complete")
  plot(hc, xlab = "", ylab = "", sub = "")
}

ex10 = function() {
  X = matrix(rnorm(20 * 3 * 50), ncol = 50)
  X[21:40,] = X[21:40,] + runif(20, -5,-1)
  X[41:60,] = X[41:60,] + runif(20, 1, 5)
  Y = c(rep(1,20), rep(2,20), rep(3, 20))
  
  X.original = X
  X = scale(X)
  pr.out = prcomp(X)
  
  plot(pr.out$x[,1], pr.out$x[,2], col = Y)
  
  km.out = kmeans(X, 3, nstart = 20)
  print(table(km.out$cluster, Y))
  
  km.out = kmeans(X, 2, nstart = 20)
  print(table(km.out$cluster, Y))
  
  km.out = kmeans(X, 4, nstart = 20)
  print(table(km.out$cluster, Y))
  
  km.out = kmeans(pr.out$x[,1:2], 3, nstart = 20)
  print(table(km.out$cluster, Y))
  
  km.out = kmeans(X.original, 3, nstart = 20)
  print(table(km.out$cluster, Y))
}

ex11 = function() {
  data = read.csv("Ch10Ex11.csv", header = F)
  
  print(sd(data[,1]))
  data = scale(data)
  print(sd(data[,1]))
  
  data = t(data)
  dis = as.dist(1 - cor(t(data)))
  hc = hclust(dis, method = "complete")
  plot(hc, xlab = "", ylab = "", sub = "")
  
  cluster = cutree(hc, 2)
  Y = c(rep(1, 20), rep(2, 20))
  print(table(cluster, Y))
  
  find.most.diff = function(cluster) {
    g1 = cluster == 1
    g2 = cluster == 2
    d = (apply(data[g1,], 2, mean) - apply(data[g2,], 2, mean))^2
    k = which.max(d)
    print(sprintf("%f gene differs the most", k))
  }
  
  find.most.diff(Y)
  find.most.diff(cluster)
}
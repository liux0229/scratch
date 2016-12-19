library(ISLR)
library(tree)
library(MASS)
library(randomForest)
library(gbm)
library(glmnet)
library(class) # KNN

# Constants definition
LINE_SOLID_LTY = 1
LINE_DASH_LTY = 3

err.rate = function(pred, Y) {
  t = table(pred, Y)
  return((t[1] + t[4]) / sum(t))
}

tree.classification = function() {
  High = ifelse(Carseats$Sales <= 8, "No", "Yes")
  Carseats.high = data.frame(Carseats, High)
  
  # Inspect the tree
  fit = tree(High ~ . - Sales, Carseats.high)
  print(summary(fit))
  # print(summary(fit))
  # print(fit)
  # plot(fit)
  # text(fit, pretty = 0)

  # Validation set approach to estimate test error
  n = nrow(Carseats)
  train = sample(1:n, n/2)
  test = -train
  
  # Note pass model = T otherwise cv.tree cannot find the X & Y data
  tree.fit = tree(High ~ . - Sales, Carseats.high, subset = train, model = T)
  print(err.rate(predict(tree.fit, Carseats.high[test,], type = "class"), High[test]))
  
  # Cross validation. Note cv.tree takes in the full tree.
  # The prune.misclass argument indicates using error rate as the metric
  # for tree pruning and cross validation, instead of the default deviance
  # (i.e. cross entropy)
  # Q: how is x-entropy used when evaluating for a single example?
  set.seed(1)
  fit.cv = cv.tree(tree.fit, FUN = prune.misclass)
  print(fit.cv)
  
  par(mfrow = c(1,2))
  plot(fit.cv$size, fit.cv$dev, type = "b")
  plot(fit.cv$k, fit.cv$dev, type = "b")
  
  best = which.min(fit.cv$dev)
  print(sprintf("Size %d tree obtains the smallest CV error rate", fit.cv$size[best]))
  
  tree.prune = prune.misclass(tree.fit, best = fit.cv$size[best])
  
  plot(tree.fit)
  text(tree.fit, pretty = 0)
  plot(tree.prune)
  text(tree.prune, pretty = 0)
  print(err.rate(predict(tree.prune, Carseats.high[test,], type="class"), High[test]))
  
  print(
    err.rate(
      predict(
        prune.misclass(tree.fit, best = fit.cv$size[best] + 5),
        Carseats.high[test,], type="class"), 
      High[test]))
}

tree.regression = function() {
  set.seed(1)
  n = nrow(Boston)
  train = sample(1:n, n/2)
  test = -train
  
  fit = tree(medv ~ ., Boston, subset = train)
  par(mfrow = c(1,1))
  plot(fit)
  text(fit, pretty = 0)
  
  fit.cv = cv.tree(fit)
  plot(fit.cv$size, fit.cv$dev, type = "b")
  print(sprintf("Full fit size: %d; cv size: %d", 
                summary(fit)$size, 
                fit.cv$size[which.min(fit.cv$dev)]))
  pred = predict(fit, newdata = Boston[test,])
  
  Y.test = Boston[test,]$medv
  plot(pred, Y.test)
  abline(0, 1) # This allows us to see how well the model is predicting
  mean((pred - Y.test)^2)
}

tree.random.forest = function () {
  set.seed(1)
  n = nrow(Boston)
  train = sample(1:n, n/2)
  test = -train
  
  bag.fit = randomForest(medv ~ ., 
                         data = Boston, 
                         subset = train, 
                         # mtry = ncol(Boston) - 1, 
                         mtry = 6,
                         importance = T,
                         ntree = 25)
  print(bag.fit)
  
  pred = predict(bag.fit, newdata = Boston[-train,])
  plot(pred, Boston[test,]$medv)
  abline(0, 1)
  mean((pred - Boston[test,]$medv)^2)
  
  print(importance(bag.fit))
  varImpPlot(bag.fit)
}

tree.boosting = function() {
  set.seed(1)
  n = nrow(Boston)
  train = sample(1:n, n/2)
  test = -train
  
  boost.fit = gbm(medv ~ ., 
                  data = Boston[train,], 
                  # regression - gaussian
                  # classification - bernoulli
                  distribution = "gaussian",
                  n.trees = 1000,
                  interaction.depth = 4,
                  # default is 0.001
                  shrinkage = 0.2)
  summary(boost.fit)
  
  # Partial dependence plot: marginal effect of a predictor to the response
  par(mfrow = c(1,2))
  plot(boost.fit, i = "rm")
  plot(boost.fit, i = "lstat")
  
  # Why do we need to pass # trees again (it's required)? 
  # Looks like this achieves the effect of "only using a subset of trees"
  pred = predict(boost.fit, newdata = Boston[test,], n.trees = 1000)
  print(mean((pred - Boston[test,]$medv)^2))
  par(mfrow = c(1,1))
  plot(pred, Boston[test,]$medv)
  abline(0, 1)
}

plot.curve.ex3 = function() {
  par(mfrow = c(1,1))
  X = seq(0, 1, 0.01)
  Y.gini = 2 * X * (1 - X)
  Y.err = ifelse(X < 0.5, X, 1 - X)
  Y.entropy = -(X * log(X) + (1 - X) * log(1 - X))
  
  plot(X, Y.gini, type = "l", col = "red", ylim = c(0, 1))
  points(X, Y.err, type = "l", col = "blue")
  points(X, Y.entropy, type = "l", col = "green")
  legend("topright", 
         legend = c("gini", "err", "x-entropy"), 
         col = c("red", "blue", "green"), 
         lty = LINE_SOLID_LTY, lwd = 2, cex = 0.8)
}

ex5 = function() {
  prob = c(0.1, 0.15, 0.2, 0.2, 0.55, 0.6, 0.6, 0.65, 0.7, 0.75)
  pred = prob > 0.5
  m = length(prob)
  pred.1 = ifelse(sum(pred) >= m / 2, "red", "green")
  pred.2 = ifelse(mean(prob) > 0.5, "red", "green")
  print(sprintf("%s %s", pred.1, pred.2))
}

ex7 = function () {
  set.seed(1)
  n = nrow(Boston)
  train = sample(1:n, n/2)
  test = -train
  
  trees = seq(1, 500, 10)
  P = ncol(Boston) - 1
  mtrys = c(P, P / 2, sqrt(P))
  mse = matrix(NA, length(mtrys), length(trees))
  for (i in 1:length(trees)) {
    for (j in 1:length(mtrys)) {
      bag.fit = randomForest(medv ~ ., 
                             data = Boston, 
                             subset = train, 
                             mtry = mtrys[j],
                             importance = T,
                             ntree = trees[i])
      pred = predict(bag.fit, newdata = Boston[test,])
      mse[j, i] = mean((pred - Boston[test,]$medv)^2)
    }
  }
  plot(trees, mse[1,], type = "l", col = "red", ylim = c(0, 25))
  points(trees, mse[2,], type = "l", col = "blue")
  points(trees, mse[3,], type = "l", col = "green")
  legend("topright", 
         legend = c("m = p", "m = p / 2", "m = sqrt(p)"),
         col = c("red", "blue", "green"),
         lty = LINE_SOLID_LTY, lwd = 2, cex = 0.8)
}

ex8 = function() {
  par(mfrow = c(1,2))
  set.seed(1)
  n = nrow(Carseats)
  train = sample(1:n, n/2)
  test = -train
  
  tree.fit = tree(Sales ~ ., Carseats, subset = train, model = T)
  plot(tree.fit)
  text(tree.fit, pretty = F)
  pred = predict(tree.fit, newdata = Carseats[test,])
  print(sprintf("full tree: %f", mean((pred - Carseats[test,]$Sales)^2)))
  
  fit.cv = cv.tree(tree.fit)
  best = which.min(fit.cv$dev)
  fit.prune = prune.tree(tree.fit, method = "deviance", best = fit.cv$size[best])
  plot(fit.prune)
  text(fit.prune, pretty = F)
  pred = predict(fit.prune, newdata = Carseats[test,])
  print(sprintf("pruned tree: %f", mean((pred - Carseats[test,]$Sales)^2)))
  
  bag.fit = randomForest(Sales ~ ., 
                         data = Carseats, 
                         subset = train, 
                         mtry = ncol(Carseats) - 1,
                         importance = T,
                         ntree = 100)
  pred = predict(bag.fit, newdata = Carseats[test,])
  print(sprintf("Bag: %f", mean((pred - Carseats[test,]$Sales)^2)))
  print(importance(bag.fit))
  varImpPlot(bag.fit)
  
  P = ncol(Carseats) - 1
  mse = rep(NA, P)
  for (m in 1:P) {
    forest.fit = randomForest(Sales ~ ., 
                              data = Carseats, 
                              subset = train, 
                              mtry = m,
                              importance = T,
                              ntree = 100)
    pred = predict(forest.fit, newdata = Carseats[test,])
    mse[m] = mean((pred - Carseats[test,]$Sales)^2)
  }
  plot(mse, type = "l", col = "red")
  best = which.min(mse)
  print(sprintf("Forest: %d %f", best, mse[best]))
}

ex9 = function() {
  set.seed(1)
  n = nrow(OJ)
  train = sample(1:n, 800)
  test = -train
  
  tree.fit = tree(Purchase ~ ., OJ, subset = train, model = T)
  plot(tree.fit)
  text(tree.fit, pretty = F)
  print(summary(tree.fit))
  
  print(tree.fit)
  
  pred = predict(tree.fit, newdata = OJ[test,], type = "class")
  print(table(pred, OJ[test,]$Purchase))
  print(mean(pred == OJ[test,]$Purchase))
  
  fit.cv = cv.tree(tree.fit, FUN = prune.tree)
  plot(fit.cv$size, fit.cv$dev, type = "b", col = "blue")
  best = which.min(fit.cv$dev) 
  print(fit.cv$size)
  print(fit.cv$dev)
  print(best)
  points(c(fit.cv$size[best]), c(fit.cv$dev[best]), pch = "x", col = "red")
  
  tree.prune = prune.misclass(tree.fit, best = fit.cv$size[best])
  print(summary(tree.prune))
  # plot(tree.prune)
  # text(tree.prune, pretty = F)
  pred = predict(tree.prune, newdata = OJ[test,], type = "class")
  print(mean(pred == OJ[test,]$Purchase))
}

ex10 = function() {
  set.seed(1)
  Hitters = na.omit(Hitters)
  Hitters$Salary = log(Hitters$Salary)
  train = 1:200
  test = -train
  
  s = seq(0.01, 0.3, 0.01)
  k = length(s)
  mse.train = rep(NA, k)
  mse.test = rep(NA, k)
  for (i in 1:k) {
    boost.fit = gbm(Salary ~ ., 
                    data = Hitters[train,], 
                    distribution = "gaussian",
                    n.trees = 1000,
                    interaction.depth = 4,
                    shrinkage = s[i])
    mse.train[i] = mean((predict(boost.fit, newdata = Hitters[train,], n.trees = 1000) - Hitters[train,]$Salary)^2)
    mse.test[i] = mean((predict(boost.fit, newdata = Hitters[test,], n.trees = 1000) - Hitters[test,]$Salary)^2)
  }
  par(mfrow = c(1,1))
  plot(s, mse.train, type = "l", col = "blue", ylim = c(0.0, 0.4))
  points(s, mse.test, type = "l", col = "red")
  print(sprintf("Boosting: %f", min(mse.test)))
  
  legend("topright", 
         legend = c("train", "test"),
         col = c("blue", "red"),
         lty = LINE_SOLID_LTY, lwd = 2, cex = 0.8)
  
  lm.fit = lm(Salary ~ ., data = Hitters, subset = train)
  print(sprintf("lm: %f", mean((predict(lm.fit, newdata = Hitters[test,]) - Hitters[test,]$Salary)^2)))
  
  fit = cv.glmnet(model.matrix(Salary ~ ., data = Hitters[train,])[,-1], 
                  Hitters[train,]$Salary, 
                  alpha = 0)
  pred = predict(fit, 
                 s = fit$lambda.min, 
                 newx = model.matrix(Salary ~ ., data = Hitters[test,])[,-1])
  print(sprintf("ridge: %f", mean((pred - Hitters[test,]$Salary)^2)))
  
  # Bagging
  bagging.fit = randomForest(Salary ~ ., 
                             data = Hitters, 
                             subset = train, 
                             mtry = ncol(Hitters) - 1,
                             importance = T,
                             ntree = 100)
  pred = predict(bagging.fit, newdata = Hitters[test,])
  print(sprintf("bagging: %f", mean((pred - Hitters[test,]$Salary)^2)))
  
  # Variable importance
  print(summary(lm.fit))
  print(summary(gbm(Salary ~ ., 
        data = Hitters[train,], 
        distribution = "gaussian",
        n.trees = 1000,
        interaction.depth = 4,
        shrinkage = s[which.min(mse.test)])))
  print(importance(bagging.fit))
}

ex11 = function() {
  train = 1:1000
  test = -train
  Caravan$Purchase = ifelse(Caravan$Purchase == 'Yes', 1, 0)
  boost.fit = gbm(Purchase ~ ., 
                  data = Caravan[train,], 
                  distribution = "bernoulli",
                  n.trees = 1000,
                  interaction.depth = 4,
                  shrinkage = 0.01)
  print(summary(boost.fit))
  
  report = function(pred.prob, method) {
    pred = ifelse(pred.prob > 0.2, 1, 0)
    t = table(pred, Caravan$Purchase[test])
    print(t)
    print(sprintf("%s: %f", method, t[2,2] / (t[2,1] + t[2,2])))
  }
  
  # response: probability
  # link: logit
  report(predict(boost.fit, newdata = Caravan[test,], n.trees = 1000, type = "response"), "boosting")
  
  # Logistic regression
  glm.fit = glm(Purchase ~ ., data = Caravan[train,], family = binomial)
  report(predict(glm.fit, newdata = Caravan[test,], type = "response"), "logistic regression")
  
  # KNN
  # Standardize the input
  X = scale(Caravan[,-86])
  # Note the order of the input
  pred.knn = knn(X[train,], X[test,], Caravan$Purchase[train], k = 5, prob = T)
  
  # Convoluted, but works
  pred.raw.prob = attr(pred.knn, "prob")
  pred.prob = ifelse(pred.knn == 1, pred.raw.prob, 1 - pred.raw.prob)
  report(pred.prob, "KNN (k = 5)")
}
library(e1071)
library(ROCR)

support.vector.classifier = function() {
  set.seed(1)
  X = matrix(rnorm(20 * 2), ncol = 2)
  Y = c(rep(-1, 10), rep(1, 10))
  # Make the 1 class drawn from norm(1, 1)
  X[Y == 1,] = X[Y == 1,] + 1
  plot(X, col = (3 - Y))
  
  # as.factor returns the variable treated as a factor variable
  # otherwise we'll perform SVM regression
  frame = data.frame(X = X, Y = as.factor(Y))
  # scale: whether to scale the predictors to have mean 0 and sd 1
  svm.fit = svm(Y ~ ., data = frame, kernel = "linear", cost = 10, scale = F)
  
  plot(svm.fit, frame)
  print(sprintf("There are %d support vectors", length(svm.fit$index)))
  print(svm.fit$index)
  print(X[svm.fit$index,])
  
  # What is the semantics of the list() call?
  # Bias - variance tradeoff:
  # The wider the margin, the larger the bias and the lower the variance
  tune.out = tune(svm, Y ~ ., data = frame, kernel = "linear", 
                  ranges = list(cost = c(0.001, 0.01, 0.1, 1, 5, 10, 100)))
  # What is error measuring? Training error or logloss?
  summary(tune.out)
  
  best.model = tune.out$best.model
  summary(best.model)
  
  X.test = matrix(rnorm(20 * 2), ncol = 2)
  Y.test = sample(c(-1, 1), 20, rep = T)
  X.test[Y.test == 1,] = X[Y.test == 1,] + 1
  frame.test = data.frame(X = X.test, Y = as.factor(Y.test))
  pred = predict(best.model, frame.test)
  print(pred)
  print(table(predict = pred, truth = Y.test))
  
  # Linearly separable case
  X[Y == 1,] = X[Y == 1,] + 0.5
  plot(X, col = (3 - Y))
  
  # data.frame() does copy; so need to repeat this
  frame = data.frame(X = X, Y = as.factor(Y))
  
  svm.fit = svm(Y ~ ., data = frame, kernel = "linear", cost = 1e5, scale = F)
  # The graph appears as if the margin does not have equal width at the two sides,
  # which could be a rendering artifact
  plot(svm.fit, frame)
  print(summary(svm.fit))
  
  X.test[Y.test == 1,] = X[Y.test == 1,] + 0.5
  frame.test = data.frame(X = X.test, Y = as.factor(Y.test))
  pred = predict(best.model, frame.test)
  # I obtained the same result for cost = 1e5 & cost = 1, could be that 
  # our test set is not large enough
  print(table(predict = pred, truth = Y.test))
}

roc.plot = function(pred, truth, ...) {
  predob = prediction(pred, truth)
  perf = performance(predob, "tpr", "fpr")
  plot(perf, ...)
}

support.vector.machine = function() {
  set.seed(1)
  X = matrix(rnorm(200 * 2), ncol = 2)
  X[1:100,] = X[1:100,] + 2
  X[101:150,] = X[101:150,] - 2
  Y = c(rep(1, 150), rep(2, 50))
  frame = data.frame(X = X, Y = as.factor(Y))
  plot(X, col = Y)
  
  train = sample(200, 100)
  test = -train
  svm.fit = svm(Y ~ ., data = frame[train,], kernel = "radial", gamma = 1, cost = 1)
  plot(svm.fit, frame[train,])
  print(svm.fit)
  
  pred = predict(svm.fit, frame[test,])
  print(table(pred = pred, truth = frame[test,]$Y))
  
  set.seed(1)
  tune.out = tune(svm, Y ~ ., data = frame, kernel = "radial", 
                  ranges = list(cost = c(0.1, 1, 10, 100, 1000, 1000),
                                gamma = c(0.01, 0.05, 0.1, 0.25, 0.5, 1, 2, 3, 4)))
  print(summary(tune.out))
  best.model = tune.out$best.model
  
  # passing newx argument makes it return the predictions for all examples
  # (including training) for some reason
  pred = predict(best.model, frame[test,])
  
  print(pred)
  print(table(pred = pred, truth = frame[test,]$Y))
  
  fit.and.plot.roc = function(pred.set, gamma, title, add = F, col = "black") {
    svm.fit.values = svm(Y ~ ., data = frame[train,], kernel = "radial", 
                         gamma = gamma, cost = 1, decision.values= T)
    fitted = attributes(predict(svm.fit.values, frame[pred.set,], decision.values = T))$decision.values
    # How does it know the sign determines the label 
    # and the magnitude can be used as probability?
    roc.plot(fitted, frame[pred.set,]$Y, main = title, add = add, col = col)
  }
  
  par(mfrow = c(1, 2))
  fit.and.plot.roc(train, 2, "Training data")
  fit.and.plot.roc(train, 50, "Test data", T, "red")
  fit.and.plot.roc(test, 2, "Training data")
  fit.and.plot.roc(test, 50, "Test data", T, "red")
  
  # Multi-class SVM
  set.seed(1)
  X = rbind(X, matrix(rnorm(50 * 2), ncol = 2))
  Y = c(Y, rep(0, 50))
  X[Y == 0, 2] = X[Y == 0, 2] + 2
  frame = data.frame(X = X, Y = as.factor(Y))
  par(mfrow = c(1,1))
  plot(X, col = Y + 1)
  
  svm.fit = svm(Y ~ ., data = frame, kernel = "radial", cost = 10, gamma = 1)
  plot(svm.fit, frame)
}

gene.expr = function() {
  frame = data.frame(X = Khan$xtrain, Y = as.factor(Khan$ytrain))
  fit = svm(Y ~ ., data = frame, kernel = "linear", cost = 10)
  print(summary(fit))
  print(table(fit$fitted, frame$Y))
  pred = predict(fit, Khan$xtest) # We don't need to form test frame here
  table(pred, Khan$ytest)
  
  tune.out = tune(svm, Y ~ ., data = frame, kernel = "linear", 
                  ranges = list(cost = c(0.001, 0.01, 0.1, 1, 2, 5, 7, 10, 100)))
  print(tune.out)
}

svm.loss = function() {
  X = seq(-5,5,0.01)
  X2 = c(X * -1, X)
  n2 = length(X2)
  Y = c(rep(-1, length(X)), rep(1, length(X)))
  svm.loss = rep(NA, n2)
  logistic.loss = rep(NA, n2)
  
  logistic = function(x) { 1 / (1 + exp(-x)) }
   
  for (i in 1:n2) {
    svm.loss[i] = max(0, 1 - X2[i])
    logistic.loss[i] = ifelse(Y[i] == 1, 
                              -log(logistic(X2[i])), # positive example
                              -log(1 - logistic(-X2[i])) # negative example
                             )
  }
  plot(X2, svm.loss, type = "l")
  points(X2, logistic.loss, type = "l", col = "red")
}

ex1 = function() {
  X1 = seq(-10, 10, 0.1)
  X2 = 3 * X1 + 1
  plot(X1, X2, type = "l")
  
  X2 = (X1 - 2) / 2
  points(X1, X2, type = "l", col = "red")
}

ex2 = function() {
  X1 = seq(-10, 10)
  X2 = seq(-10, 10, 0.01)
}

run.svm = function(frame) {
  n = dim(frame)[1]
  print(sprintf("There are %d examples in the dataset", n))
  train = sample(n, n / 2)
  test = -train
  
  err.rate = function(dataset, case) {
    pred = predict(svm.fit, frame[dataset,])
    t = table(pred, frame[dataset,]$Y)
    print(sprintf("Error rate for %s is %f", case, (t[1] + t[4]) / sum(t)))
  }
  
  svm.fit = svm(Y ~ ., data = frame[train,], kernel = "linear", cost = 1)
  plot(svm.fit, frame[train,])
  err.rate(train, "Linear training")
  err.rate(test, "Linear test")
  Sys.sleep(5)
  
  svm.fit = svm(Y ~ ., data = frame[train,], kernel = "radial", gamma = 1, cost = 1)
  plot(svm.fit, frame[train,])
  err.rate(train, "Radial training")
  err.rate(test, "Radial test")
  Sys.sleep(5)
  
  svm.fit = svm(Y ~ ., data = frame[train,], kernel = "polynomial", degree = 2, cost = 1)
  plot(svm.fit, frame[train,])
  err.rate(train, "Polynomial training")
  err.rate(test, "Polynomial test")
}

ex4 = function() {
  # Data generation
  set.seed(1)
  X = matrix(rnorm(200 * 2), ncol = 2)
  
  # Move to the upper right
  X[1:50,] = X[1:50,] + 2
  # Move to the lower left
  X[51:100,] = X[51:100,] - 2
  # Move to the upper left
  X[101:125,1] = X[101:125,1] - 3
  X[101:125,2] = X[101:125,2] + 3
  # Move to the lower right
  X[126:150,1] = X[126:150,1] + 3
  X[126:150,2] = X[126:150,2] - 3
  
  Y = c(rep(1, 150), rep(2, 50))
  frame = data.frame(X = X, Y = as.factor(Y))
  plot(X, col = Y)
  
  run.svm(frame)
}

ex4.2 = function() {
  n = 500
  X1 = runif(n) - 0.5
  X2 = runif(n) - 0.5
  Y = ifelse(X1 ^ 2 - X2 ^ 2 > 0, 1, -1)
  X = cbind(X1, X2)
  frame = data.frame(Y = as.factor(Y), X = X)
  run.svm(frame)
}

ex5 = function() {
  n = 500
  X1 = runif(n) - 0.5
  X2 = runif(n) - 0.5
  Y = 1 * (X1 ^ 2 - X2 ^ 2 > 0)
  frame = data.frame(Y = Y, X1 = X1, X2 = X2)
  plot(X1, X2, col = Y + 1)
  train = sample(n, n / 2)
  test = -train
  
  
  fit = glm(Y ~ ., data = frame[train,], family = binomial)
  pred = predict(fit, frame[train,], type = "response")
  pred.label = pred > 0.5
  plot(X1[train], X2[train], col = pred.label + 1)
  
  fit = glm(Y ~ X1 + X2 + I(X1^2) + I(X2^2) + X2 * X2, 
            data = frame[train,], family = binomial)
  pred = predict(fit, frame[train,], type = "response")
  pred.label = pred > 0.5
  plot(X1[train], X2[train], col = pred.label + 1)
  
  Y = ifelse(Y == 1, 1, -1)
  frame = data.frame(Y = as.factor(Y), X1 = X1, X2 = X2)
  fit = svm(Y ~ ., data = frame[train,], kernel = "linear", cost = 1)
  pred.label = predict(fit, frame[train,])
  plot(X1[train], X2[train], 
       col = (pred.label == 1) + 2, 
       pch = ifelse(Y[train] == 1, 3, 5))
  
  fit = svm(Y ~ ., data = frame[train,], kernel = "radial", gamma = 1, cost = 1)
  pred.label = predict(fit, frame[train,])
  plot(X1[train], X2[train], 
       col = (pred.label == 1) + 2, 
       pch = ifelse(Y[train] == 1, 3, 5))
}

ex6 = function() {
  set.seed(1)
  n = 100
  X = matrix(rnorm(n * 2), ncol = 2)
  Y = c(rep(-1, n / 2), rep(1, n / 2))
  X[Y == 1,] = X[Y == 1,] + 2.8
  plot(X, col = (3 - Y))
  # return(1)
  
  frame = data.frame(X = X, Y = as.factor(Y))
  costs = c(0.001, 0.001, 0.01, 0.1, 1, 5, 10, 100, 1000, 1e4, 1e5)
  tune.out = tune(svm, Y ~ ., data = frame, kernel = "linear", 
                  ranges = list(cost = costs))
  
  C = length(costs)
  err.train = rep(NA, C)
  err.cv = tune.out$performances$error
  err.test = rep(NA, C)
  
  X.test = matrix(rnorm(n * 2), ncol = 2)
  Y.test = c(rep(-1, n / 2), rep(1, n / 2))
  X.test[Y.test == 1,] = X.test[Y.test == 1,] + 2.8
  
  for (i in 1:C) {
    fit = svm(Y ~ ., data = frame, kernel = "linear", cost = costs[i])
    pred = predict(fit, frame)
    t = table(pred, Y)
    # print(t)
    err.train[i] = (t[2] + t[3]) / sum(t)
    
    pred = predict(fit, data.frame(X = X.test, Y = as.factor(Y.test)))
    t = table(pred, Y.test)
    err.test[i] = (t[2] + t[3]) / sum(t)
  }
  
  plot(log(costs), err.train, type = "l", col = "red", ylim = c(0, 0.2))
  points(log(costs), err.cv, type = "l", col = "blue")
  points(log(costs), err.test, type = "l", col = "green")
  
  print(err.train)
  print(sprintf("Best cost for training is: %f", costs[which.min(err.train)]))
  print(sprintf("Best cost for cv is: %f", costs[which.min(err.cv)]))
  print(sprintf("Best cost for test is: %f", costs[which.min(err.test)]))
}

ex7 = function() {
  med = median(Auto$mpg)
  Auto$high = ifelse(Auto$mpg > med, 1, -1)
  
#   tune.out = tune(svm, high ~ . - mpg, data = Auto, kernel = "linear", 
#                   ranges = list(cost = c(1e-3, 1e-2, 1e-1, 1, 5, 10, 100, 1000)))
#   print(summary(tune.out))
#   
#   tune.out = tune(svm, high ~ . - mpg, data = Auto, kernel = "radial", 
#                   ranges = list(
#                     cost = c(1e-3, 1e-2, 1e-1, 1, 5, 10, 100, 1000),
#                     gamma = c(0.1, 0.5, 1, 2, 3, 4, 5)))
#   print(summary(tune.out))
#   
#   tune.out = tune(svm, high ~ . - mpg, data = Auto, kernel = "polynomial", 
#                   ranges = list(
#                     cost = c(1e-3, 1e-2, 1e-1, 1, 5, 10, 100, 1000),
#                     degree = c(1, 2, 3, 4, 5)))
#   print(summary(tune.out))
  
  fit = svm(high ~ . - mpg, data = Auto, kernel = "radial", gamma = 0.1, cost = 5)
  
  # Copied from the solution
  plotpairs = function(fit) {
    for (name in names(Auto)[!(names(Auto) %in% c("mpg", "mpglevel", "name"))]) {
      plot(fit, Auto, as.formula(paste("mpg~", name, sep = "")))
    }
  }
  
  # Our plotting function does not seem to work
  plotpairs(fit)
}

ex8 = function() {
  n = nrow(OJ)
  train = sample(1:n, 800)
  test = -train
  
  run.svm = function(kernel) {
    err.rate = function(dataset, case) {
      pred = predict(fit, OJ[dataset,])
      t = table(pred, OJ[dataset,]$Purchase)
      print(sprintf("Error rate for %s is %f", case, (t[2] + t[3]) / sum(t)))
    }
    
    fit = svm(Purchase ~ ., OJ[train,], kernel = kernel, cost = 0.01, degree = 2)
    # print(summary(fit))
    
    err.rate(train, sprintf("%s train cost = 0.01", kernel))
    err.rate(test, sprintf("%s test cost = 0.01", kernel))
    
    costs = c(0.01, 0.015, 0.02, 0.05, 0.1, 0.2, 0.5, 1, 2, 5, 10)
    tune.out = tune(svm, Purchase ~ ., data = OJ[train,], kernel = kernel, degree = 2,
                    ranges = list(cost = costs))
    best.cost = tune.out$best.parameters$cost
    
    fit = svm(Purchase ~ ., OJ[train,], kernel = "linear", cost = best.cost)
    err.rate(train, sprintf("%s train cost = %f (optimal)", kernel, best.cost))
    err.rate(test, sprintf("%s test cost = %f (optimal)", kernel, best.cost))
  }
  
  run.svm("linear")
  run.svm("radial")
  run.svm("polynomial")
}
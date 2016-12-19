report = function(predicted.label, Y) {
  print(sprintf("Overall correct rate: %f", 
        mean(predicted.label == Y[!train])));
  mtx = table(predicted.label, Y[!train])
  print(sprintf("Recall:%f specificity:%f", 
                mtx[2,2] / (mtx[1,2] + mtx[2,2]),
                mtx[1,1] / (mtx[1,1] + mtx[2,1])))
}

report_glm = function(fit, X, Y, pos = 'Up', neg = 'Down') {
  pred.prob = predict(fit, X[!train,], type="response")
  pred.label = rep(neg, length(pred.prob))
  pred.label[pred.prob > 0.5] = pos
  report(pred.label, Y)
}

report_lda = function(fit, X, Y) {
  pred.label = predict(fit, X[!train,])$class
  report(pred.label, Y)
}

predict.regsubsets = function(model.fit, data, size) {
  # Predict using the model with size `size` on data
  # mat = model.matrix(as.formula(model.fit$call[[2]]), data)
  mat = model.matrix(Salary ~ ., data) # Temporary hack
  c = coef(model.fit, id = size)
  return(mat[,names(c)] %*% c)
}

regsubsets.k.folds = function() {
  k = 10
  folds = sample(1:k, nrow(Hitters), replace = T)
  cv.errors = matrix(NA, k, 19, dimnames = list(NULL, paste(1:19)))
  for (j in 1:k) {
    # Perform best subsets
    fit = regsubsets(Salary ~ ., data = Hitters[folds != j,], nvmax = 19)
    # Computes cv MSE for each model size
    for (i in 1:19) {
      test = folds == j
      pred = predict.regsubsets(fit, Hitters[test,], i)
      cv.errors[j, i] = mean((pred - Hitters$Salary[test])^2)
    }
  }
  return(apply(cv.errors, 2, mean))
}

generateX = function() {
  X = matrix(NA, 1000, 20)
  for (i in 1:20) {
    X[,i] = rnorm(1000, mean = i, sd = 0.1)
  }
  return (X)
}

generateB = function() {
  B = rep(0, 20)
  for (i in 20:1) {
    if (i %% 2 == 0) {
      B[i] = i
    }
  }
  return (B)
}

doPredict = function() {
  B = generateB()
  
  # par(mfrow = c(2,1))
  col_names = colnames(X, do.NULL = F, prefix = "x.")
  best.fit = regsubsets(y ~ ., data = data.frame(x = x_train, y = y_train), nvmax = 20)
  best.fit.s = summary(best.fit)
  print(which.min(best.fit.s$rss))
  plot(best.fit.s$rss / 100, type = "b", pch = 19)
  mse = rep(0, 20)
  diff = rep(0, 20)
  
  for (i in 1:20) {
    co = coef(best.fit, id = i)
    sel = col_names %in% names(co)
    pred = model.matrix(y ~ ., data = data.frame(x[,sel], y)) %*% co
    mse[i] = mean((pred - y)^2)
    
    B_est = rep(0, 20)
    B_est[sel] = co[-1]
    diff[i] = sqrt(sum((B - B_est)^2))
  }
  points(mse, type = "b", pch = 19, col = "red")
  print(which.min(mse))
  # par(mfrow = c(1,1))
  coef(best.fit, id = 10)
  
  return (diff)
}
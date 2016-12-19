# Load necessary libraries and data sets
library(ISLR)
library(splines)
library(MASS) # Boston
library(leaps) # regsubsets
library(gam)

# Constants definition
LINE_SOLID_LTY = 1
LINE_DASH_LTY = 3

find_upper_bound = function(x, t) {
  for (i in 1:length(x)) {
    if (!is.na(x[i]) && x[i] >= t) {
      return (i)
    }
  }
  return (length(x) + 1)
}

polynomial = function() {
  # 1. First, predict for wage using age
  
  fit = lm(wage ~ poly(age, 4), data = Wage)
  print(coef(summary(fit)))
  
  fit.raw = lm(wage ~ poly(age, 4, raw = T), data = Wage)
  print(coef(summary(fit.raw)))
  
  lim = range(Wage$age)
  age.grid = seq(from = lim[1], to = lim[2])
  
  # What does the `list' construct mean below? When should we use newx vs. newdata?
  # Requesting se: for every input point, we also compute the standard error of the response
  # Note if we pass the se argument, we are effectively requesting multiple output
  # from the predict() function, and thus we need to access the $fit & $se.fit
  # output separately.
  pred = predict(fit, newdata = list(age = age.grid), se = T)
  se.bands = cbind(pred$fit + 2 * pred$se.fit, pred$fit - 2 * pred$se.fit)
  
  # mar: margin on the four sides (bottom, left, top, right). 
  # Does not include title.
  # oma: outer margin; includes title.
  par(mfrow = c(1, 2), mar = c(4.5, 4.5, 1, 1), oma = c(0, 0, 4, 0))
  
  # cex: how large each point is
  plot(Wage$age, Wage$wage, xlim = lim, cex = .5, col = "darkgrey")
  
  title("Degree-4 Polynomial", outer = T)
  
  # lwd stands for `line width'
  lines(age.grid, pred$fit, lwd = 2, col = "blue")
  
  # matlines means we are drawing for multiple lines simultaneously, where
  # the Y value for each X is in each column of the matrix on the corresponding row
  matlines(age.grid, se.bands, lwd = 1, col = "blue", lty = LINE_DASH_LTY)
  
  pred.raw = predict(fit.raw, newdata = list(age = age.grid), se = T)
  print(sum((pred$fit - pred.raw$fit)^2))
  
  # How complex the polynomial regression need to be?
  fit.1 = lm(wage ~ age, data = Wage)
  fit.2 = lm(wage ~ poly(age, 2), data = Wage)
  fit.3 = lm(wage ~ poly(age, 3), data = Wage)
  fit.4 = lm(wage ~ poly(age, 4), data = Wage)
  fit.5 = lm(wage ~ poly(age, 5), data = Wage)
  anova.ret = anova(fit.1, fit.2, fit.3, fit.4, fit.5)
  
  # Check the relationship between F statistic from the anova call to the
  # t-statistic from the coefficients of the highest degree model (and 
  # thus the correspondence between the F probability and the t probability)
  print(anova.ret$F)
  print((coef(summary(fit.5))[,3])^2)
  
  choose_degree = function(ret) {
    # We cannot use degree `ret', so `ret' - 1 is what we want
    degree = find_upper_bound(ret$`Pr(>F)`, 0.05) - 1
    
    print(sprintf("Polynomial of degree %d is chosen", degree))
  }
  choose_degree(anova.ret)
  
  fit.m.1 = lm(wage ~ education + age, data = Wage)
  fit.m.2 = lm(wage ~ education + poly(age, 2), data = Wage)
  fit.m.3 = lm(wage ~ education + poly(age, 3), data = Wage)
  choose_degree(anova(fit.m.1, fit.m.2, fit.m.3))
  
  # Second, predict for Pr(wage > 250K) using age
  glm.fit = glm(I(wage > 250) ~ poly(age, 4), data = Wage, family = binomial)
  # By default, type = 'link', which stands for compute the logit (X * beta)
  glm.pred.logit = predict(glm.fit, newdata = list(age = age.grid), se = T)
  
  # Once we have the prediction and bands for logit, 
  # we can translate them into probabilities.
  # We cannot use type = 'response' directly to obtain the standard errors
  # as that would have produced negative probabilities
  logistic = function(x) {
    return (1 / (1 + exp(-x)))
  }
  glm.bands.logit = cbind(glm.pred.logit$fit + 2 * glm.pred.logit$se.fit, 
                          glm.pred.logit$fit - 2 * glm.pred.logit$se.fit)
  glm.pred = logistic(glm.pred.logit$fit)
  glm.bands = logistic(glm.bands.logit)
  
  # Plot the RHS graph
  # type = 'n' indicates no-plotting (just setting up the dimension of the plot)
  plot(Wage$age, I(Wage$wage > 250), xlim = lim, type = "n", ylim = c(0, .2))
  # jitter separates points with the same age slightly (rug plot)
  points(jitter(Wage$age), I((Wage$wage > 250) / 5), cex = .5, pch = "|", col = "darkgrey")
  lines(age.grid, glm.pred, lwd = 2, col = "blue")
  matlines(age.grid, glm.bands, lwd = 1, col = "blue", lty = LINE_DASH_LTY)
  
  par(mfrow = c(1,1))
}

step_func = function() {
  fit = lm(wage ~ cut(age, 4), data = Wage)
  summary(fit)
  lim = range(Wage$age)
  age.grid = seq(from = lim[1], to = lim[2])
  pred = predict(fit, newdata = list(age = age.grid), se = T)
  se.bands = cbind(pred$fit - 2 * pred$se.fit, pred$fit + 2 * pred$se.fit)
  
  plot(Wage$age, Wage$wage, xlim = lim, cex = .5, col = "darkgrey")
  lines(age.grid, pred$fit, lwd = 2, col = "blue")
  matlines(age.grid, se.bands, lwd = 1, col = "blue", lty = LINE_DASH_LTY)
}

plot_fitted_curve = function(X, pred, pt.X = Wage$age, pt.Y = Wage$wage) {
  plot(pt.X, pt.Y, col = "gray", cex = .5)
  lines(X, pred$fit, lwd = 2)
  lines(X, pred$fit + 2 * pred$se.fit, lty = LINE_DASH_LTY)
  lines(X, pred$fit - 2 * pred$se.fit, lty = LINE_DASH_LTY)
}

reg_spline = function() {
  library(splines)
  par(mfrow = c(2,2))
  
  # Regression spline
  # df = 6 produces 3 knots when by default we cubic splines
  # What's exactly the semantics of `degree' for bs()?
  # Using a shared `spline' variable does not work for some reason
  print(attr(bs(Wage$age, df = 6), "knots"))
  # spline.fit = lm(wage ~ bs(age, knots = c(25, 40, 60)), data = Wage)
  spline.fit = lm(wage ~ bs(age, df = 6), data = Wage)
  
  lim = range(Wage$age)
  age.grid = seq(from = lim[1], to = lim[2])
  plot_fitted_curve(age.grid,
                    predict(spline.fit, newdata = list(age = age.grid), se = T))
  
  # Natural regression spline
  # I need to understand how many knots does bs() & ns() produce with a particular
  # df argument
  print(attr(ns(Wage$age, df = 6), "knots"))
  ns_spline.fit = lm(wage ~ ns(age, df = 6), data = Wage)
  plot_fitted_curve(age.grid,
                    predict(ns_spline.fit, newdata = list(age = age.grid), se = T))

  # Smooth spline
  smooth.spline.fit = smooth.spline(Wage$age, Wage$wage, df = 16)
  smooth.spline.fit.cv = smooth.spline(Wage$age, Wage$wage, cv = T)
  print(smooth.spline.fit.cv$df)
  
  plot(Wage$age, Wage$wage, col = "gray", cex = .5)
  # We could directly plot a curve for smooth splines
  lines(smooth.spline.fit, col = "red", lwd = 2)
  lines(smooth.spline.fit.cv, col = "blue", lwd = 2)
  legend("topright", legend = c("16 DF", "6.8 DF"), col = c("red", "blue"), 
         lty = LINE_SOLID_LTY, lwd = 2, cex = 0.8)
  
  # Local regression; could also use locfit
  local.fit = loess(wage ~ age, span = .2, data = Wage)
  local.fit2 = loess(wage ~ age, span = .5, data = Wage)
  plot(Wage$age, Wage$wage, col = "gray", cex = .5)
  lines(age.grid, predict(local.fit, data.frame(age = age.grid)), col = "red", lwd = 2)
  lines(age.grid, predict(local.fit2, data.frame(age = age.grid)), col = "blue", lwd = 2)
  legend("topright", legend = c(".2 span", ".5 span"), col = c("red", "blue"), 
         lty = LINE_SOLID_LTY, lwd = 2, cex = 0.8)
}

gam_reg = function() {
  library(gam)
  
  par(mfrow = c(1,3))
  gam.fit = lm(wage ~ ns(age, 5) + ns(year, 4) + education, data = Wage)
  
  # We can use plot.gam to plot for a fit produced by least squares
  # How to interpret the plot: suppose we fit y = f1(x1) + f2(x2) + f3(x3),
  # then the plots display f1, f2 & f3 respectively. It could also be 
  # interpreted as what happens to y we vary x1 while holding x2 & x3 constant.
  
  plot.gam(gam.fit, se = T, color = "red")
  
  gam.smooth.m3 = gam(wage ~ s(age, 5) + s(year, 4) + education, data = Wage)
  plot(gam.smooth.m3, se = T, col = "green")
  
  # ANOVA test
  gam.smooth.m1 = gam(wage ~ s(age, 5) + education, data = Wage)
  gam.smooth.m2 = gam(wage ~ year + s(age, 5) + education, data = Wage)
  # Notice m2 is considered to consist of a subset of predictors from m3
  print(anova(gam.smooth.m1, gam.smooth.m2, gam.smooth.m3))
  
  # The p-value is corresponding to the null hypothesis that 
  # there is a linear relationship between the response and the predictor
  summary(gam.smooth.m3)
  
  print(mean((Wage$wage - predict(gam.smooth.m1))^2))
  print(mean((Wage$wage - predict(gam.smooth.m2, newdata = Wage))^2))
  print(mean((Wage$wage - predict(gam.smooth.m3, newdata = Wage))^2))
}

gam_reg_local_regression = function() {
  par(mfrow = c(1,3))
  
  # With degree = 1, span = .7 does not really give a linear fit
  gam.lo = gam(wage ~ s(year, df = 4) + lo(age, span = .7, degree = 1) + education, data = Wage)
  plot(gam.lo, se = T, col = "green")
  
  # gam.lo.i = gam(wage ~ lo(year, age, span = .5) + education, data = Wage)
  # library(akima)
  # plot(gam.lo.i)
}

gam_lr = function() {
  par(mfrow = c(1,3))
  gam.lr = gam(I(wage > 250) ~ year + s(age, df = 5) + education, 
               family = binomial,
               data = Wage)
  plot(gam.lr, se = T, col = "green")
  
  print(table(Wage$education, I(Wage$wage > 250)))
  gam.lr = gam(I(wage > 250) ~ year + s(age, df = 5) + education, 
               family = binomial,
               data = Wage[Wage$education != '1. < HS Grad',])
  plot(gam.lr, se = T, col = "green")
}

# Exercises
poly.regression.7.6 = function() {
  par(mfrow = c(1,2))
  library(boot)
  set.seed(1)
  
  # Polynomial regression
  N = 10
  err = rep(NA, N)
  for (d in 1:N) {
    fit = glm(wage ~ poly(age, d), data = Wage)
    err[d] = cv.glm(Wage, fit, K = 10)$delta[1]
  }
  
  # Degree 4 is chosen by cross CV and ANOVA chose degree 3.
  best_d = which.min(err)
  
  fit = glm(wage ~ poly(age, best_d), data = Wage)
  
  lim = range(Wage$age)
  age.grid = seq(from = lim[1], to = lim[2])
  plot_fitted_curve(age.grid,
                    pred = predict(fit, newdata = list(age = age.grid), se = T))
  
  # Step function
  # Need to redo this part by correctly implementing k-folds
  N = 8
  err = rep(NA, N)
  for (k in 2:N) {
    fit = glm(wage ~ cut(age, k), data = Wage)
    err[k] = cv.glm(Wage, fit, K = 10)$delta[1]
  }
  best_k = which.min(err)
  fit = glm(wage ~ cut(age, best_k), data = Wage)
  plot_fitted_curve(age.grid,
                    predict(fit, newdata = list(age = age.grid), se = T))
}

poly.regression.7.7 = function() {
  par(mfrow = c(1,2))
  plot(Wage$maritl, Wage$wage)
  plot(Wage$jobclass, Wage$wage)
  
  # Deviance can be understood as similar to RSS
  print(deviance(lm(wage ~ maritl, data = Wage)))
  print(deviance(lm(wage ~ jobclass, data = Wage)))
  print(deviance(gam(wage ~ s(age, 4), data = Wage)))
  # jobclass & maritl do add explanation power
  print(deviance(gam(wage ~ jobclass + maritl + s(age, 4), data = Wage)))
}

poly.regression.7.8 = function() {
  par(mfrow = c(2,2))
  fit = lm(mpg ~ poly(horsepower, 4), data = Auto)
  lim = range(Auto$horsepower)
  grid = seq(from = lim[1], to = lim[2])
  plot_fitted_curve(grid,
                    predict(fit, newdata = list(horsepower = grid), se = T),
                    Auto$horsepower, Auto$mpg)
  
  fit = lm(mpg ~ bs(horsepower, df = 4), data = Auto)
  plot_fitted_curve(grid,
                    predict(fit, newdata = list(horsepower = grid), se = T),
                    Auto$horsepower, Auto$mpg)
  
  fit = lm(mpg ~ ns(horsepower, df = 4), data = Auto)
  plot_fitted_curve(grid,
                    predict(fit, newdata = list(horsepower = grid), se = T),
                    Auto$horsepower, Auto$mpg)
  
  fit = smooth.spline(Auto$horsepower, Auto$mpg, df = 4)
  lines(fit, col = "red", lwd = 2)
}

poly.regression.7.8.2 = function() {
  err.poly = rep(NA, 20)
  err.bs = rep(NA, 20)
  err.ns = rep(NA, 20)
  err.step = rep(NA, 20)
  for (d in 1:20) {
    err.poly[d] = cv.glm(Auto, glm(mpg ~ poly(horsepower, d), data = Auto))$delta[1]
    err.bs[d] = cv.glm(Auto, glm(mpg ~ bs(horsepower, d), data = Auto))$delta[1]
    err.ns[d] = cv.glm(Auto, glm(mpg ~ ns(horsepower, d), data = Auto))$delta[1]
    if (d > 1) {
      Auto$horsepower.cut = cut(Auto$horsepower, d)
      err.step[d] = cv.glm(Auto, glm(mpg ~ horsepower.cut, data = Auto))$delta[1]
    }
  }
  print(sprintf("Polynomial degree of %d is chosen", which.min(err.poly)))
  print(sprintf("Regression spline df of %d is chosen", which.min(err.bs)))
  print(sprintf("Natural spline df of %d is chosen", which.min(err.ns)))
  print(sprintf("Step function with %d cuts is chosen", which.min(err.step)))
}

ex.7.9.1 = function() {
  par(mfrow = c(5,2))
  lim = range(Boston$dis)
  grid = seq(from = lim[1], to = lim[2])
  
  D = 10
  rss = rep(NA, D)
  err = rep(NA, D)
  for (d in 1:D) {
    fit = glm(nox ~ poly(dis, d), data = Boston)
    plot_fitted_curve(grid,
                      predict(fit, newdata = list(dis = grid), se = T),
                      Boston$dis, Boston$nox)
    rss[d] = sum(summary(fit)$deviance.resid^2)
    err[d] = cv.glm(Boston, fit)$delta[1]
  }
  print(rss)
  print(sprintf("Polynomial degree of %d is chosen", which.min(err)))
}

ex.7.9.2 = function() {
  par(mfrow = c(5,4))
  lim = range(Boston$dis)
  grid = seq(from = lim[1], to = lim[2])
  
  D = 20
  rss = rep(NA, D)
  err = rep(NA, D)
  for (d in 1:D) {
    fit = glm(nox ~ bs(dis, d), data = Boston)
    plot_fitted_curve(grid,
                      predict(fit, newdata = list(dis = grid), se = T),
                      Boston$dis, Boston$nox)
    rss[d] = sum(summary(fit)$deviance.resid^2)
    err[d] = cv.glm(Boston, fit)$delta[1]
  }
  print(rss)
  print(sprintf("Regression spline df of %d is chosen", which.min(err)))
}

ex.7.10 = function() {
  set.seed(1)
  par(mfrow = c(4,2))
  n = dim(College)[1]
  train = sample(n, n / 2)
  test = -train
  
  # Variable selection
  r = regsubsets(Outstate ~ ., data = College[train,])
  s = summary(r)
  
  # Note the solution uses 0.2 std of the minimum statistic
  best = which.min(s$cp)
  print(sprintf("Optimal # predictors = %d", best))
  coef(r, id = best)
  
  gam = gam(Outstate ~ Private + 
                       s(Accept, df = 2) + 
                       s(Enroll, df = 2) + 
                       s(Room.Board, df = 2) + 
                       s(PhD, df = 2) + 
                       s(perc.alumni, df = 2) + 
                       s(Expend, df = 2) + 
                       s(Grad.Rate, df = 2), 
            data = College[train,])
  plot(gam, se = T, col = "green")
  
  rss = sum(College[test,]$Outstate - predict(gam, newdata = College[test,]))^2
  tss = sum(College[test,]$Outstate - mean(College[train,]$Outstate))^2
  print(sprintf("R^2 for GAM: %f", (tss - rss) / tss))
  
  print(sprintf("R^2 for least squared: %f",
                summary(lm(Outstate ~ Private + Accept + Enroll + Room.Board + PhD + perc.alumni + Expend + Grad.Rate, 
                        data = College, subset = train))$r.squared))
  
  summary(gam)
}

ex.7.11 = function() {
  par(mfrow = c(1,1))
  set.seed(1)
  n = 100
  X1 = rnorm(n, 10, 1)
  X2 = rnorm(n, 20, 5)
  beta0 = 5
  beta1 = 2
  beta2 = 7
  Y = beta0 + beta1 * X1 + beta2 * X2 + rnorm(n, 0, 10)
  
  # Fit using backfitting
  K = 3
  b0 = rep(NA, K)
  b1 = rep(NA, K)
  b2 = rep(NA, K)
  b0[1] = b1[1] = b2[1] = 0
  for (k in 2:K) {
    t = Y - X1 * b1[k - 1]
    b2[k] = lm(t ~ X2)$coef[2]
    
    t = Y - X2 * b2[k]
    fit = lm(t ~ X1)
    b1[k] = fit$coef[2]
    b0[k] = fit$coef[1]
  }
  
  print(sprintf("%f %f %f", b0[K], b1[K], b2[K]))
  plot(b0, type = "l", col = "green", ylim = c(-1, 10))
  # Add lines to an existing plot
  lines(b1, col = "red")
  lines(b2, col = "blue")
  
  # multiple regression
  fit = lm(Y ~ X1 + X2)
  print(fit$coefficients)
  
  # Plot the multiple regression line using abline (e.g. horizontal line)
  abline(h = fit$coefficients[1], lty = LINE_DASH_LTY)
  abline(h = fit$coefficients[2], lty = LINE_DASH_LTY)
  abline(h = fit$coefficients[3], lty = LINE_DASH_LTY)
}

ex.7.12 = function() {
  par(mfrow = c(1,1))
  set.seed(1)
  n = 150
  p = 100
  X = matrix(NA, nrow = n, ncol = p)
  for (i in 1:p) {
    X[,i] = rnorm(n, i, 5)
  }

  beta = rep(NA, p)
  for (i in 1 : p) {
    beta[i] = p - i + 1
  }
  Y = 4 + X %*% beta + rnorm(n, 0, 1)
  
  K = 100
  b0 = rep(0, K)
  b = matrix(0, nrow = p, ncol = K)
  for (k in 2:K) {
    # copies values from the previous iteration
    b[,k] = b[,k - 1]
    
    for (i in 1:p) {
      # compute residual
      t = Y - X %*% b[,k] + X[,i] * b[i, k]
      fit = lm(t ~ X[,i])
      b[i, k] = fit$coef[2]
      b0[k] = fit$coef[1]
    }
  }
  
  print(b0[K])
  print(b[,K])
  
  plot(b0, type = "l", col = "green", ylim = c(-20, 130))
  for (i in 1:p) {
    lines(b[i,], col = i)
  }
  
  fit = lm(Y ~ X)
  print(fit$coefficients)
}
import sys
from sys import stdin

def program():
	n, k = [int(x) for x in stdin.readline().split()]
	numbers = [int(x) for x in stdin.readline().split()]
	print(solve(n, k, numbers))

def solve(n, k, numbers):
	primes = getPrimes(k)
	# print(primes)
	visited = set()
	for x in numbers:
		g = gcd(k, x)
		generate(g, primes, visited, 0)
	total = 1
	for (p, c) in primes:
		total *= c + 1
	total -= len(visited)
	return total

def generate(d, primes, visited, np):
	if d in visited:
		return False
	visited.add(d)
	if d == 1:
		return True
	for i in xrange(np, len(primes)):
		current = d
		p = primes[i][0]
		while current % p == 0:
			current //= p
			if not generate(current, primes, visited, i + 1):
				break
	return True

def getPrimes(k):
	primes = []
	k = tryPrime(k, 2, primes)
	for p in xrange(3, int(k ** 0.5), 2):
		k = tryPrime(k, p, primes)
	if k > 1:
		primes.append((k, 1))
	return primes

def tryPrime(k, p, primes):
	count = 0
	while k % p == 0:
		k /= p
		count = count + 1
	if count > 0:
		primes.append((p, count))
	return k

def gcd(a, b):
	return a if b == 0 else gcd(b, a % b)

program()

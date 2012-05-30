#include <cstdio>
#include <cmath>
#include <cassert>
#include <list>
#include <vector>
#include <algorithm>
using namespace std;

class HashTable
{
public:
	// creates a hash table which can store divisors of n
	HashTable(long long n) : n_(n), count_(0)
	{
		int maxi = static_cast<int>(sqrt(static_cast<double>(n))) + 1; // +1 to avoid precision loss
		table_.resize(maxi + 1);
	}

	void add(long long d)
	{
		// whether d*d <= n, or d <= n^0.5
		if (d <= n_ / d)
		{
			unsigned pos = static_cast<unsigned int>(d);
			// assert(pos < table_.size());
			if (!table_[pos].first)
				++count_;
			table_[pos].first = true;
		}
		else
		{
			unsigned pos = static_cast<unsigned int>(n_ / d);
			// assert(pos < table_.size());
			if (!table_[pos].second)
				++count_;
			table_[pos].second = true;
		}
	}

	bool has(long long d) const
	{
		if (d <= n_ / d)
			return table_[static_cast<unsigned int>(d)].first;
		else
			return table_[static_cast<unsigned int>(n_ / d)].second;
	}

	int count() const
	{
		return count_;
	}

private:
	long long n_;
	vector<pair<bool, bool>> table_;
	int count_;
};

class Solver
{
public:
	Solver(vector<long long> const& numbers, long long k);
	int solve();
private:
	void getPrimeFactors(long long n);
	void tryPrime(long long & n, long long p);
	long long gcd(long long a, long long b) const;
	bool enumerate(long long d, int nextPrime);

	vector<long long> const& numbers_;
	long long k_;
	HashTable table_;
	vector<pair<long long, int>> primes_;
};

Solver::Solver(vector<long long> const& numbers, long long k)
	: numbers_(numbers),
	  k_(k),
	  table_(k)
{
	getPrimeFactors(k);
}

int Solver::solve()
{
	for (size_t i = 0; i < numbers_.size(); i++)
	{
		long long g = gcd(k_, numbers_[i]);
		// enumerate all divisors of g
		enumerate(g, 0);
	}

	// what's the total number of divisors for k_?
	int total = 1;
	for (size_t i = 0; i < primes_.size(); i++)
	{
		total *= (primes_[i].second + 1);
	}

	return total - table_.count();
}

bool Solver::enumerate(long long d, int nextPrime)
{
	// if d has been generated before, then the 'parent' who generated d must have generated d's all proper divisors
	if (table_.has(d))
	{
		return false;
	}

	table_.add(d);

	if (d == 1)
	{
		return true;
	}

	for (int i = nextPrime; i < static_cast<int>(primes_.size()); i++)
	{
		long long current = d;
		long long p = primes_[i].first;
		while (current % p == 0)
		{
			current /= p;
			if (!enumerate(current, i + 1))
			{
				break;
			}
		}
	}

	return true;
}

void Solver::getPrimeFactors(long long n)
{
	vector<pair<long long, int>> primes;
	tryPrime(n, 2);
	for (long long i = 3; i * i <= n; i += 2)
	{
		tryPrime(n, i);
	}
	if (n > 1)
	{
		primes_.push_back(make_pair(n, 1));
	}
}

void Solver::tryPrime(long long & n, long long p)
{
	int count = 0;
	while (n % p == 0)
	{
		++count;
		n /= p;
	}
	if (count > 0)
	{
		primes_.push_back(make_pair(p, count));
	}
}

long long Solver::gcd(long long a, long long b) const
{
	return b == 0 ? a : gcd(b, a % b);
}

int main()
{
	int n;
	long long k;
	scanf("%d %lld", &n, &k);
	vector<long long> numbers;
	numbers.reserve(n);
	for (int i = 0; i < n; i++)
	{
		long long a;
		scanf("%lld", &a);
		numbers.push_back(a);
	}

	printf("%d\n", Solver(numbers, k).solve());
}
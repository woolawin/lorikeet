
build:
	bazel build --repo_env=CC=clang++ --cxxopt='-std=c++20' //src:lorikeet

clean:
	bazel clean --expunge

test:
	bazel test --repo_env=CC=clang++ --cxxopt='-std=c++20' --test_output=all //src:test
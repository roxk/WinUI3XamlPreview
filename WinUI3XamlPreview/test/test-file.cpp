namespace foo {
	struct bar {};
}

int main() {
	TARGET-NAMESPACE::bar baz;
	return 0;
}

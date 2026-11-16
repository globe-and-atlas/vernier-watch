test:
	python3 -m pytest -q
build:
	python3 execution/build.py
verify:
	python3 execution/verify.py
fixture:
	python3 execution/build_fixture.py

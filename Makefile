VENV_DIR = venv

.PHONY: default
default: run

.PHONY: install
install:
	nix develop

run:
	@echo "Starting FastAPI server..."
	$(VENV_DIR)/bin/uvicorn main:app --reload

.PHONY: format
format:
	ruff check --fix .
	ruff format .

# Lint the Python code using ruff
.PHONY: lint
lint:
	ruff check .

.PHONY: test
test:
	pytest

# Clean up virtual environment and cache
.PHONY: clean
clean:
	@echo "Cleaning up..."
	rm -rf $(VENV_DIR)
	rm -rf __pycache__
	rm -rf .pytest_cache


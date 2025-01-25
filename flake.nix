{
  description = "Kryptori environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
    let
      pkgs = import nixpkgs { inherit system; };
      python = pkgs.python312;
    in
    rec {
      devShell = pkgs.mkShell {
        buildInputs = [
          python
          python.pkgs.setuptools
          python.pkgs.wheel
          python.pkgs.uv
        ];

        shellHook = ''
          echo "Entering Python development environment..."
          export PYTHONPATH=$PWD
          # Create a virtualenv inside the shell if it doesn't exist
          if [ ! -d "venv" ]; then
            uv venv venv
          fi

          # Activate the virtual environment
          source venv/bin/activate

          # Install dependencies from requirements.txt
          if [ -f requirements.txt ]; then
            uv pip install -r requirements.txt
          fi

          echo "All dependencies installed via uv."
        '';
      };

      packages.default = python.pkgs.buildPythonPackage {
        pname = "kryptori";
        version = "0.1.0";

        src = ./.;

        propagatedBuildInputs = [];

        # Optional: If you need to run tests
        # checkPhase = ''
        #   pytest tests/
        # '';
      };
    });
}


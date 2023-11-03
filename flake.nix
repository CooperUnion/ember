{
  description = "Ember";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
        };

        python = pkgs.python311;

      in
      {
        devShells.default = pkgs.mkShell {
          packages = [
            python
            pkgs.nixpkgs-fmt
          ];
        };
      }
    );
}

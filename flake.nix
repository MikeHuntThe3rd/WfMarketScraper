{
  description = "Dev shell that matches my system";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
  let
    system = "x86_64-linux";
    pkgs = import nixpkgs {
      inherit system;
      config.allowUnfree = true;
    };
  in
  {
    devShells.${system}.default = pkgs.mkShell {
      packages = with pkgs; [
	curl
        # extra dev-only deps
        cmake
        pkg-config
      ];

      shellHook = ''
        echo "Dev shell loaded"
      '';
    };
  };
}


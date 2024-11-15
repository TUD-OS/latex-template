# Nix Flake exporting an attribute to build the thesis in a Nix derivation.
# Also exproting a dev shell to get the relevant tooling to build the
# thesis traditionally on the shell.
#
# Make sure to configure your `nix` with:
# `experimental-features = flakes nix-command`
# in order to be able to use flakes.
#
# Feel free to ping "@phip1611" for maintenance guidance and questions.
{
  description = "TUD OS LaTex Template";

  inputs = {
    flake-parts.url = "github:hercules-ci/flake-parts";
    flake-parts.inputs.nixpkgs-lib.follows = "nixpkgs";
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
  };

  outputs =
    { self, ... }@inputs:
    inputs.flake-parts.lib.mkFlake { inherit inputs; } {
      flake = { };
      systems = inputs.nixpkgs.lib.systems.flakeExposed;

      perSystem =
        {
          system,
          pkgs,
          ...
        }:
        {
          devShells.default = pkgs.mkShell {
            inputsFrom = [ self.packages.${system}.thesis ];
            packages = with pkgs; [
              nixfmt-rfc-style
            ];
          };

          # for `nix fmt .`
          formatter = pkgs.nixfmt-rfc-style;

          packages =
            let
              texToolchain = pkgs.callPackage ./nix/tex-toolchain.nix { };
              thesis = pkgs.callPackage ./nix/thesis.nix {
                tex = texToolchain;
              };
            in
            {
              default = thesis;
              inherit thesis;
            };
        };
    };
}

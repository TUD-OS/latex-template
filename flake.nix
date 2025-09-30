# Nix Flake exporting an attribute to build the thesis in a Nix derivation.
# Also exporting a dev shell to get the relevant tooling to build the
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
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
  };

  outputs =
    { self, ... }@inputs:
    let
      # Systems definition for dev shells and exported packages,
      # independent of the NixOS configurations and modules defined here. We
      # just use "every system" here to not restrict any user. However, it
      # likely happens that certain packages don't build for/under certain
      # systems.
      systems = inputs.nixpkgs.lib.systems.flakeExposed;
      forAllSystems =
        function:
        inputs.nixpkgs.lib.genAttrs systems (system: function inputs.nixpkgs.legacyPackages.${system});
    in
    {

      devShells.default = forAllSystems (
        pkgs:
        pkgs.mkShell {
          inputsFrom = [ self.packages.${pkgs.system}.thesis ];
          packages = with pkgs; [
            nixfmt-rfc-style
          ];
        }
      );

      # for `nix fmt .`
      formatter = forAllSystems (pkgs: pkgs.nixfmt-rfc-style);

      packages =
        forAllSystems (
          pkgs:
          let
            texToolchain = pkgs.callPackage ./nix/tex-toolchain.nix { };
            thesis = pkgs.callPackage ./nix/thesis.nix {
              tex = texToolchain;
            };
          in
          {
            default = thesis;
            inherit thesis;
          }
        );
    };
}

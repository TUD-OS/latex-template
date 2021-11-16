let
  sources = import ./sources.nix;
  pkgs = import sources.nixpkgs {};

  inherit (import sources."gitignore.nix" { inherit (pkgs) lib; }) gitignoreSource;
in
{
  latex-template = pkgs.callPackage ./build.nix {
    inherit gitignoreSource;
  };
}

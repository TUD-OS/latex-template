{ sources ? import ./sources.nix
, pkgs ? import sources.nixpkgs { }
}:
let
  tex = import ./tex-toolchain.nix { inherit pkgs; };
  inherit (import sources."gitignore.nix" { inherit (pkgs) lib; }) gitignoreSource;
in
{
  pdf = pkgs.callPackage ./build.nix {
    inherit gitignoreSource;
    inherit tex;
  };
}

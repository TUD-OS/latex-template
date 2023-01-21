{ sources ? import ./nix/sources.nix
, pkgs ? import sources.nixpkgs { }
}:

let
  tex = import ./nix/tex-toolchain.nix { inherit pkgs; };
in
pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    gnumake
    tex

    # convenience for working with nix files
    niv
    nixpkgs-fmt
  ];
}

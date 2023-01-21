{ sources ? import ./nix/sources.nix
, pkgs ? import sources.nixpkgs { }
}:
let
  thisPackage = import ./nix/release.nix {
    inherit sources;
    inherit pkgs;
  };
in
pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    # convenience for working with nix files
    niv
    nixpkgs-fmt
  ];

  inputsFrom = [
    thisPackage.pdf
  ];
}

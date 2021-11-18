{ stdenvNoCC, gitignoreSource, texlive }:

stdenvNoCC.mkDerivation {
  pname = "latex-template";
  version = "1.0.0";

  src = gitignoreSource ../.; 

  nativeBuildInputs = [
    # This can probably be stripped down.
    texlive.combined.scheme-full
  ];

  doConfigure = false;

  # Avoid luatex failing due to non-writable cache.
  TEXMFVAR = "/tmp/texlive/";
  
  buildPhase = ''
    make
  '';

  installPhase = ''
    mkdir -p $out/share/latex-template
    install -m 0644 diplom.pdf $out/share/latex-template/
  '';
}

with import <nixpkgs> {}; {
  WriterPi = stdenv.mkDerivation {
    name = "Writer Pi";
    buildInputs = [ gcc gnumake logkeys ];
  };
}

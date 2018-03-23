with import <nixpkgs> {}; {
  WriterPi = stdenv.mkDerivation {
    name = "WriterPi";
    buildInputs = [ gcc gnumake logkeys ];
  };
}

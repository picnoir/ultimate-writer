with import <nixpkgs> {}; {
  ExHack = stdenv.mkDerivation {
    name = "ExHack";
    buildInputs = [ gcc gnumake logkeys ];
  };
}

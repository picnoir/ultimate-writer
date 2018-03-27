with import <nixpkgs> {}; {
  UltimateWriter = stdenv.mkDerivation {
    name = "ultimatewriter";
    buildInputs = [ gcc gnumake logkeys ];
  };
}

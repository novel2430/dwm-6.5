{
  description = "DWM Flake";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
  
  let 
    pkgs = nixpkgs.legacyPackages.x86_64-linux;
    libs = with pkgs; with pkgs.xorg; [
        libX11
        pkg-config
        gnumake
        libX11
        libXft
        libXinerama
        pango
        yajl

        # libinput
        # libxcb
        # libxkbcommon
        # pixman
        # wayland
        # wayland-protocols
        # wayland-scanner
        # libX11
        # xcbutilwm
        # xwayland
        # pkg-config
        # 
        # wlroots_0_19
        # tllist
        # fcft
        # pixman
        # libdrm
        # dbus
        # my-wlroots
      ];
    # my-wlroots = pkgs.wlroots.overrideAttrs (final: prev : with pkgs; rec {
    #   src = fetchFromGitLab {
    #     domain = "gitlab.freedesktop.org";
    #     owner = "wlroots";
    #     repo = "wlroots";
    #     rev = "f36f856cdb2266cf7773e3d1b5bd2ebba2ceecf5";
    #     hash = "sha256-N+fWEP+K24eme45EAfSqaK1WCQrRGussfrSBzJiW+44=";
    #   };
    # });
  in {
    devShell.x86_64-linux = pkgs.mkShell {
      packages = [];
      buildInputs = libs;
      LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath libs;
    };
  };
}

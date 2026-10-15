class QTipUikit < Formula
  desc "A mod for Q-Tip that provides UI"
  homepage "https://github.com/c-plusplus-dev/homebrew-q-tip-uikit"
  url "https://github.com/c-plusplus-dev/homebrew-q-tip-uikit/archive/refs/tags/v0.0.3.tar.gz"
  head "https://github.com/c-plusplus-dev/homebrew-q-tip-uikit.git", branch: "main"
  sha256 "7754ca2f69c45a1ba192a58f9531caa4cc5fc238f90c8dbcac998a63aeeaad1a"
  license "MIT"

  depends_on "cmake"
  depends_on "magic_enum"
  depends_on "c-plusplus-dev/q-tip/q-tip"

  def install
    system "cmake", "-S", ".", "-B", "build",
            "-DCMAKE_BUILD_TYPE=Release",
           *std_cmake_args

    system "cmake", "--build", "build"

    system "cmake", "--install", "build"
  end
end
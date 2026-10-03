class Scinumtools3 < Formula
  desc "C++ toolkit for unit-aware scientific computation"
  homepage "https://github.com/vrtulka23/scinumtools3"
  url "https://github.com/vrtulka23/scinumtools3/archive/refs/tags/v0.9.0.tar.gz"
  sha256 "dd8e47093f844da6f3c5f2aea4afc786ab581040d311972e27755b9879edfce5"
  license "MIT"
  head "https://github.com/vrtulka23/scinumtools3.git", branch: "main"

  depends_on "cmake" => :build
  depends_on "ninja" => :build
  depends_on "cpp-httplib" => :build
  depends_on "hdf5"

  resource "briefpp" do
    url "https://github.com/vrtulka23/briefpp/archive/624aa478149a0fa0e7213bb7cfb6d19615d6e771.tar.gz"
    sha256 "aaac2a8a25f8a7bc11546038594eb9fef43bf560c0c0532a886c432d5e14de31"
  end

  def install
    resource("briefpp").stage do
      (buildpath/"external/briefpp").mkpath
      cp_r "include", buildpath/"external/briefpp"
    end

    args = std_cmake_args + %w[
      -GNinja
      -DENABLE_UNIT_TESTS=OFF
      -DENABLE_BINDING_PYTHON=OFF

      -DENABLE_CORE=ON
      -DENABLE_EXS=ON
      -DENABLE_VAL=ON
      -DENABLE_PUQ=ON
      -DENABLE_DIP=ON
      -DENABLE_MAT=OFF
      -DENABLE_API=ON

      -DENABLE_EXEC_APPS=ON
      -DENABLE_EXEC_APPS_SNT=ON
      -DENABLE_SNT_SERVER=ON
      -DENABLE_SNT_VIEW=OFF
      -DENABLE_SNT_DMAP=ON
      -DENABLE_EXEC_EXAMPLES=OFF
      -DENABLE_EXEC_BENCHMARKS=OFF
    ]

    args << "-DSNT_HTTPLIB_INCLUDE_DIR=#{Formula["cpp-httplib"].opt_include}"

    system "cmake", "-S", ".", "-B", "build", *args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end

  test do
    output = shell_output("#{bin}/snt -v")
    assert_match version.to_s, output
    assert_match "Usage: snt server", shell_output("#{bin}/snt server --help")
    assert_match "Usage: snt dmap", shell_output("#{bin}/snt dmap --help")
  end
end

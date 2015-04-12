#!/usr/bin/perl -w

use strict;
use File::Copy "mv"; 
use File::Find; 
use File::Path;
use Tie::File;

# --  subroutines -- 
sub usage;
sub substitute_by_type;
sub substitute_projname;
sub make_dir_structure;
sub add_includes;
sub str2file;


# -- main -- 

# autoflush
$|++;
# nobody else can read the files we are creating
umask 077;

# arguments
usage() unless @ARGV == 2;
my ($NAME, $TYPE) = @ARGV;
die "Bad project type: $TYPE"
    unless ($TYPE eq "so" or $TYPE eq "lib");
my @PARTS = split /\./, $NAME;

# actual work
make_dir_structure($NAME);
add_includes($NAME, @PARTS);
substitute_projname($NAME);
substitute_by_type($NAME, $TYPE);
make_task_skeleton($NAME, @PARTS) if $TYPE eq "so";

# -- helper routines --

sub usage {
    die "usage: $0 <module_name> <so|lib>\n";
}

# make a skeleton task for the plugin version
sub make_task_skeleton {
    my $name  = shift;
    my @parts = @_;
    my $file  = "./$name/source/SampleTask.cpp";

    # go parts -1 deep w/ the namespacing
    my @cut = @parts[0..$#parts-1];
    my $namespace_start = join "",
                          map { "namespace $_\n{\n" }
                          @cut;
    my $namespace_end   = join "",
                          map { "}\n"; }
                          @cut;

    # set up the namespace qualified name
    my $qual_name = join "::", (@cut, ucfirst $parts[-1]);

    # set up the 
    my $uc_qual_name = uc $qual_name;
    $uc_qual_name =~ s/::/_/g;

    # Nice class name for the task
    my $task_name = ucfirst $parts[-1];

    my %pats = (
        __NAMESPACE_START__   => $namespace_start,
        __NAMESPACE_END__     => $namespace_end,
        __QUAL_NAME__         => $qual_name,
        __TASK_NAME_UC_QUAL__ => $uc_qual_name,
        __TASK_NAME__         => $task_name
    );

    substitute_pats($file, %pats);
    mv($file, "./$name/source/$task_name.cpp");
}

# setup stuff for .a or for .so
sub substitute_by_type {
    my $name = shift;
    my $type = "." . shift();

    # File::Find
    find(sub
    {
        # use the sample task for so files
        unlink $_ if ($_ eq "SampleTask.cpp" && $type eq ".lib");
        
        # filter out files that don't concern us
        return unless /^(.*make.*(?:.so|.lib))$/ig;

        my $name = $1;
        # if the name matches the type
        #   set the file to the canonical one
        #   (ie project.make.in.so => project.make.in)
        # if not
        #   nuke the file
        if ($name =~ /^(.*)$type$/g) {
            my $tmp = $1;
            mv($name, $tmp);
        }
        else {
            unlink $name;
        }
    },
    "./$name");
}

# substitute the project name into project.make.in file
sub substitute_projname {
    my $name = shift;
    my $file = "./$name/build/project.make.in";
    my %pats = (__NAME => $name);
    substitute_pats($file, %pats);
}

# do some variable substitution on a file
sub substitute_pats {
    my $file = shift;
    my %pats = @_;
    my @file = ();
    tie @file, 'Tie::File', $file
        or die "$! could not read $file!!";
    for my $line (@file) {
        for my $pat (keys %pats) {
           $line =~ s/$pat/$pats{$pat}/g;
        }
    }
    untie @file;
}

# create the include directories
sub add_includes {
    my $root  = shift;
    my @parts = @_;

    # nuke the existing directory structure
    rmtree("./$root/include", { verbose => 0 });

    # make the directory structure for the imports
    my $include_prefix = "./$root/include/";

    # create the import dirs
    my @import_dirs = ();
    push @import_dirs, $include_prefix . "import/" . join "/", @parts[0..$#parts-1];
    push @import_dirs, $include_prefix . join "/", @parts;

    # make the directories
    map{ mkpath($_, { verbose => 1 , mode => 0700 }) } @import_dirs;

    # create the import header
    my $define = join "_", map {uc} @parts;
    $define = "__${define}_H__";
    my $import_hdr = "#ifndef $define\n" .
                     "#define $define\n" .
                     "#endif  // $define\n";

    # create the file
    str2file($import_dirs[0] . "/" . $parts[-1] . ".h", $import_hdr);
}

# put a string into a file
sub str2file {
    my $file = shift;
    my $str  = shift;

    open my $fh, ">", $file
        or die "$! could not open $file for write";

    print $fh $str;

    close $fh
        or die "$! could not close $file";
}


# dump the __DATA__ section to a temporary file,
# and extract it to the current directory
sub make_dir_structure {
    my $name    = shift;
    my $tarFile = $name . ".tgz";

    open my $fh, ">", $tarFile
        or die "$! couldn't open $tarFile for write";

    # write the file as binary data
    local $/;
    local $\;
    binmode($fh);
    my $tmp = <DATA>;
    syswrite $fh, $tmp;
    close $fh
        or die "$! couldn't close $tarFile after write";

    system("tar -xzf ./$tarFile");
    unlink "./$tarFile";
    mv("./module_template", "./$name");
}


# Begin Binary Data
__DATA__
� i`�H �\�o�ϯ]����V���5H���������	-������&AU�o3^/�@HX�KߵE�kόǞ����`�cߠCFM�P=��(T���P.�+�����<_�^>�+��B�t��˗
��)�Ƭ��1�%dOQ��f�&�����d���T�J'�A���`u��K�M�/������a���=���/�����D�g�{6 �f{@'�>9i՛�ލ�\@y�ۙ��t�HJK���S���A�sK5u�#Ro���nY��lw����BA�0Hx�G=�^ұ(�V��E���]��?i5I����^��d ���E�т�f�h��T�ҭ):���:/ωnMl�T�n[Ğl�m0�=����L�ȈR�\�c0Dٌ���(�k���	pXLW�͉j-���R�#�5�.�5۠DE9�S��fPq)�4�5:�'���T5�Zf��9q@޶�Q�"W3۠�iGu�>�����٢�+Wg����{�$'�mnh;h��ڌ@�<��
����y�o�a*8ʆ�����0Mu�;�k�ǈ�SJ]��G��U-��"�ϒc����	�d̳��$�yfK�*îP]�&�-��ǔdF�n��oP�eM�B�����@.Y<�ѭ��~�yl�0�0�0�E+���
� �j�����:�G�?7�$�b�
Q1rqH���2�z�S�WU�hc�?���	�3bJy2,��Z�R�篲���I2��8""=ҭE�1��n�Q�zU�iZ� ��ԇy<(u.���R�<Y�ǎu�A{`���Ǻ[;@v,�:y�:�7>֏[iH~j����n��`��RV�N��Z%ȭ<1��8�8���D��A�eL�0�05dq]�w���s���0,<"Ț���l��|	�{7���_�V�_��H�����\^��Ǡ��m��}�:����R���_8�m_���+��C���$���_�����p��w��y�-��~5�9Λ��}�}�f�&ty����T�֭��҈ ]���V��H5�N�������N���&�p�f��>��C9�촻������B`��N�a��Z��!�k�wiu���_Pvao���R4����1�8����������۟�a$kt����vu��2y�/姀����
�t�z_������r�(��R%W�����$��Ǡ��,��n:���}ov����Vf���q��-< і��J��8���B�ٳ��S��pZ���A�G���j�$,����?�Wd8���8c/�_�;C�wv��8����>lA[�������Pa�\��!@���G�����.Uc��T��T	򫨑 ���<6�V"XӗB�6u7M�� ���SCP�-��|\�.�a��k��5~!a�x�d���W��M�g�Ƞ�p��BH¼��&N�B�1�����*��;��-+;8 ��Z೓V����ʴY�^k�a��`��WX�K��5O)���)uS��[꺶�b�5��O ^�,��W�;17�����iu� C�Ё�J���X�r��&�sݍ����u�%dyE4d��AH6��KaM��E��B>�b�-%�ؘ�v{��5���c]���Q^s>��^t:���GQ_�RG{Χ�	 ��Xw�]�hζ�Wo)�M.�0�� >�M��Zo�v�'$�?�%ob{���O��y�Y�j��9U��]1��`��]�8���8��Y�ͺukW���&�1��+��AW�}-7���ǫ|	�8��1$E�r��w.���a���Y�5l7[�A����m1u��%=mq�c���q�˕��b�_�T���1H����_��'|5�G��\F�&����d�0�B�r����)I�`Z�W�a<��O_��"��'�YT��N�+AJ�J�Kx��g+�^����4��/Nt������;Ń�j�~�C�&;����lј[�~�j%�i�s��֑��g������|��t�������p����J���|^��>
I�/��ψ���� �w#�����[��z�5 <��KbX�6��N������X���d^#5x�af����m����]_�t�� ���>�
���_�l���/xe�����E�+˴��aOp�J���;@��CI��t�!�B�th<Eb�z�#��&�5��f�{�k�+�������(���U� 7C� ���%py����֦"ަ�I{6���v��Ĭ�C`��K�l�k��>8A�u�a	ŌqT6�n�~��. ���_�,B���cG�&j�]b?��Ј�a�N� ��{7���q��{h/����I0���V��s�[�����n�w���;�_%��O(!�ïI��$���="��Ի���C����D��@4�%�Eh. H�6��C�c$!ƌ˗�qc�$~�?ތ���x�o�߁�J�|��J�B�����?����\wS��KB��W���@NN�a���Q�B�q��%����~B���E��f����P����)1�����KKWPe����/S5f;C���W+�;MY~���TW�/˗"7ȱ�C�쵹��n�"�P&�����jL�V�6�0wl�\�ܖ�g��}
P|k����Kb���%7�K��3��`}ږ۰��=ݺC,�Ok�-�ܖ5|����c�T����x����D#�>Z�ື�i��u'����IZ�d����f�	�w��m��N���a'isG���c'�p٭��Ie'Y�7n��H�aZ�52������r6r�f"��x�x���F<�.�&	�U�� �E
�/L�>��K|}��r>���9��A�?ʅr����|�T�������+`w��W>���Bv�{X��������<��������!�k������%��W�U�'&��l����G���W�P��_�$I�$I�$I�$I�$I�$I�$I�$I�$I�$I�$Iږ����_ x  
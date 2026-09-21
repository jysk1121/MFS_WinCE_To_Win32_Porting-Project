# from : https://stackoverflow.com/questions/38876945/copying-and-merging-directories-excluding-certain-extensions
from shutil import copy2, copystat, Error, ignore_patterns
import os
import tempfile
import tarfile
import zipfile
import shutil
from distutils.dir_util import copy_tree

TRANSLATION_FILES = ['APText.dat', 'DigitalMint.dat', 'JustCash.dat', 'CashDepot.dat', 'MasterScreenDesc.dat', 'MultiText.dat', 'MultiTextKeyMgr.dat', 'OPText.dat']
KEYMGR_FILES = ['KeyMgrText.dat', 'MultiTextKeyMgr.dat']


def copytree_multi(src, dst, symlinks=False, ignore=None):
    names = os.listdir(src)
    if ignore is not None:
        ignored_names = ignore(src, names)
    else:
        ignored_names = set()

    # -------- E D I T --------
    # os.path.isdir(dst)
    if not os.path.isdir(dst):
        os.makedirs(dst)
    # -------- E D I T --------

    errors = []
    for name in names:
        if name in ignored_names:
            continue
        srcname = os.path.join(src, name)
        dstname = os.path.join(dst, name)
        try:
            if symlinks and os.path.islink(srcname):
                linkto = os.readlink(srcname)
                os.symlink(linkto, dstname)
            elif os.path.isdir(srcname):
                copytree_multi(srcname, dstname, symlinks, ignore)
            else:
                copy2(srcname, dstname)
        except Error as err:
            errors.extend(err.args[0])
        except (IOError, os.error) as why:
            errors.append((srcname, dstname, str(why)))
    try:
        copystat(src, dst)
    except WindowsError:
        pass
    except OSError as why:
        errors.extend((src, dst, str(why)))
    if errors:
        raise Error(errors)


class WinCEBuilder:
    """
    Builds the Master.zip package for a specific model and version
    """

    def __init__(self, territory, version, lineage, output_dir,
                 source_dir, wince_ver=60):
        self.territory = territory
        self.version = version
        self.lineage = lineage
        self.source_dir = source_dir
        self.output_dir = output_dir
        self.wince_ver = wince_ver

        self.build_output_dir = os.path.join(self.source_dir, f'ExeRele_{wince_ver}_{territory}')

    def package_zip(self, models=[], clean=True):
        """
        Packages the build files, master files, etc. into a release package
        for the models provided.

        """

        model_string = ','.join((m.model_name for m in models))
        print(f'Building Master.zip for WinCE{self.wince_ver}: {self.lineage}{self.territory}{self.version} - {model_string}...')

        if clean:
            shutil.rmtree(self.output_dir)
            os.makedirs(self.output_dir)

        # Gather the files into a temp dir
        staging_dir = self._prepare_staging_dir(models)

        # ZIP them all into the output dir
        zip_output = os.path.join(self.output_dir, f'Master_{self.wince_ver}_{model_string}')
        self._zip_files(staging_dir, zip_output)

        # Build installation zips
        simple_version_name = self.version.replace('.', '')
        if len(models) > 1:
            zipname = os.path.join(self.output_dir, f'V{self.territory}_WINCE{self.wince_ver}_{self.lineage}{simple_version_name}_MasterOnline.zip')
        else:
            zipname = os.path.join(self.output_dir, f'V{self.territory}_WINCE{self.wince_ver}_{self.lineage}{simple_version_name}_{models[0].model_name}_MasterOnline.zip')

        # Always overwrite the master file
        if os.path.isfile(zipname):
            os.remove(zipname)

        # Apply ZIP_DEFLATED & lv.9 to tens of kilobytes, but little slower
        with zipfile.ZipFile(zipname, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as install_zip:
            if self.wince_ver == '60':
                masterfile_name = "Master.zip"
                master_path = os.path.join("update", "nh2700ce", masterfile_name)
            else:
                masterfile_name = "Master7.zip"
                master_path = os.path.join("update7", masterfile_name)
            install_zip.write(zip_output + '.zip', master_path)

        zip_path = zip_output + '.zip'
        if len(models) == 1:
            # Move Master.zip to a model-specific dir
            model_dir = os.path.join(self.output_dir, self.wince_ver, models[0].model_name)
            if not os.path.isdir(model_dir):
                os.makedirs(model_dir)

            dst_zip_path = os.path.join(model_dir, masterfile_name)
            shutil.move(zip_path, dst_zip_path)
        else:
            # Moved to the "combined" dir
            combined_dir = os.path.join(self.output_dir, self.wince_ver, 'combined')
            if not os.path.isdir(combined_dir):
                os.makedirs(combined_dir)

            dst_zip_path = os.path.join(combined_dir, masterfile_name)
            shutil.move(zip_path, dst_zip_path)

        # Remove the staging dir
        shutil.rmtree(staging_dir)

        print(model_string + ' done')
        return zip_output

    def _zip_files(self, staging_dir, output_zip):
        """
        Creates an installation zip file from the staged files

        """
        with zipfile.ZipFile(output_zip + '.zip', 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as master_zip:
            for folder, _, files in os.walk(staging_dir):
                for file in files:
                    full_name = os.path.join(folder, file)
                    master_zip.write(full_name, os.path.relpath(full_name, staging_dir))

    def _prepare_staging_dir(self, models=[]):
        """
        Moves all necessary files into a temp directory

        """
        output_dir = tempfile.mkdtemp()

        # Combine build products and master files
        self._stage_build_output(output_dir)

        # Prepare and copy the screen assets
        screen_dir = os.path.join(output_dir, "SCREEN")
        self._stage_screen_assets(screen_dir, models)

        # Copy other items
        self._stage_misc_files(output_dir)

        # Merge ETC. directory (Customer customizations)
        self._merge_customer_customization(output_dir)

        # Write updateinfo.dat
        update_info = os.path.join(output_dir, "updateinfo.dat")

        update_text = f'AP Master {self.territory} {self.lineage}{self.version} - WinCE{self.wince_ver}'
        if len(models) == 1:
            update_text += f' {models[0].model_name}'

        with open(update_info, 'w') as update:
            update.write(update_text)

        return output_dir

    def _merge_customer_customization(self, output_dir):
        """
        Copies the necessary customer files from the etc dir

        """
        data_source_dir = os.path.join(self.source_dir, "DAT", "Etc", self.territory, self.lineage)
        if not os.path.isdir(data_source_dir):
            return

        copy_tree(data_source_dir, output_dir)

    def _stage_translations(self, output_dir):
        """
        Copies the translations Text.dat to the output dir

        """
        screen_source = os.path.join(self.source_dir, 'DAT', 'SCREEN')

        for filename in TRANSLATION_FILES:
            src = os.path.join(screen_source, filename)
            shutil.copy(src, output_dir)

    def _stage_screen_assets(self, output_dir, models):
        """
        Copies all neccesssary screen files to the staging dir

        """
        # Build necessary screen assets
        screen_temp = tempfile.mkdtemp()
        for model in models:
            model.build_assets(self.territory, screen_temp, self.source_dir)

        # Translations
        self._stage_translations(screen_temp)

        # Copy over screens in a tar file
        if not os.path.isdir(output_dir):
            os.makedirs(output_dir)
        screen_output = os.path.join(output_dir, 'screen.tar')
        self._make_tar(screen_temp, screen_output)
        shutil.rmtree(screen_temp)

        # Copy key manager dat files for compatibility
        screen_source = os.path.join(self.source_dir, 'DAT', 'SCREEN')
        for filename in KEYMGR_FILES:
            src = os.path.join(screen_source, filename)
            shutil.copy(src, output_dir)

    def _stage_misc_files(self, output_dir):
        """
        Stages EMVINI, WAVE, TTS_DB, and ERRORCODE

        """
        # WAVE
        wave_output = os.path.join(output_dir, "WAVE")
        wave_input = os.path.join(self.source_dir, "DAT", "Wave", self.territory)
        copy_tree(wave_input, wave_output)

        # EMVINI
        aidlist_input = os.path.join(self.source_dir, "DAT", "EmvIni", "AIDLIST.ini")
        terdata_input = os.path.join(self.source_dir, "DAT", "EmvIni", "TERM_DATA_POOL.ini")
        territory_emv_input = os.path.join(self.source_dir, "DAT", "EmvIni", self.territory)
        emv_output = os.path.join(output_dir, "EMVINI")
        if not os.path.isdir(emv_output):
            os.makedirs(emv_output)

        shutil.copy(aidlist_input, emv_output)
        shutil.copy(terdata_input, emv_output)
        copy_tree(territory_emv_input, emv_output)

        # TTS_DB
        tts_output = os.path.join(output_dir, "TTS_DB")
        tts_input = os.path.join(self.source_dir, "Dat", "TTS_DB", self.territory)
        if os.path.isdir(tts_input):
            copy_tree(tts_input, tts_output)

        # ERRORCODE
        ec_output = os.path.join(output_dir, "ERRORCODE")
        ec_input = os.path.join(self.source_dir, "DAT", "ErrorCode")
        copy_tree(ec_input, ec_output)

        # CA Certs
        ca_output = os.path.join(output_dir)
        ca_input = os.path.join(self.source_dir, "DAT", "Certs")
        copy_tree(ca_input, ca_output)

    def _make_tar(self, screen_dir, output):
        """
        Packages the screen files into a TAR

        """
        with tarfile.open(output, 'w:') as tar:
            tar.add(screen_dir, recursive=True, arcname='')

    def _package_combine_zip(self):
        """
        Packages a Master.zip for all models except the 1500SE.

        """

        output_dir = self._combine_build_output()

    def _stage_build_output(self, output_dir):
        """
        Builds the binary packages without the screen assets, which are model-specific

        @returns the path to the output

        """
        # Copy build output
        copy_tree(self.build_output_dir, output_dir)

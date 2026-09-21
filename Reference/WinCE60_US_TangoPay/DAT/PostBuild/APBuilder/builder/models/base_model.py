import os
import shutil
import glob

DAT = 'DAT'
SCREEN = 'Screen'

DEFAULT_TERRITORY = 'US'

KNOWN_CUSTOMIZATIONS = ['pai', 'DigitalMint', 'DigitalMintOp', 'JustCash', 'JustCashOp', 'CashDepotOp']


def copy_glob(src, dst):
    if not os.path.isdir(dst):
        os.mkdir(dst)
    for file in glob.glob(src):
        shutil.copy(file, dst)


class BaseModel:
    """
    A base model builder. 

    """

    def __init__(self, model, asset_sizes, model_name):
        self.model = model
        self.asset_sizes = asset_sizes
        self.model_name = model_name

    def _get_model_image_dirs(self, source_dir, model, territory):
        """
        Returns the directory of the screen images for the specified model.

        Usually, the US territory is used as the default base. Other territories are merged ontop of it

        """
        dirs = []

        dirs.append(os.path.join(source_dir, DAT, SCREEN, model, DEFAULT_TERRITORY))

        terr_dir = os.path.join(source_dir, DAT, SCREEN, model, territory)
        if os.path.isdir(terr_dir):
            dirs.append(terr_dir)

        return dirs

    def _get_model_asset_dir(self, source_dir, model):
        """
        Returns the directory of the assets for the specified model.

        """

        return os.path.join(source_dir, DAT, SCREEN, model)

    def get_op_dat_path(self, source_dir):
		# OP.DAT is merged into one now
        # model_dir = self._get_model_asset_dir(source_dir, self.model)
        model_dir = self._get_model_asset_dir(source_dir, "")
        return os.path.join(model_dir, "Op.dat")

    def get_ap_dat_path(self, source_dir, territory):
        model_dir = self._get_model_asset_dir(source_dir, self.model)
        ap_terr = os.path.join(model_dir, territory, "Ap.dat")
        if os.path.isfile(ap_terr):
            return ap_terr

        return os.path.join(model_dir, DEFAULT_TERRITORY, "Ap.dat")

    def get_other_files(self):
        """
        Returns a list of other misc. files, which aren't contemplated by the 
        standard build process.

        """

        files = []

        for customization in KNOWN_CUSTOMIZATIONS:
            override_file = os.path.join(DAT, SCREEN, self.model, f"{customization}.dat")
            if os.path.exists(override_file):
                files.append((override_file, self.model))

        return files

    def get_keymgr_dat_path(self, source_dir):
        model_dir = self._get_model_asset_dir(source_dir, self.model)
        return os.path.join(model_dir, "KeyMgr.dat")

    
    def build_assets(self, territory, output_dir, source_dir):
        """
        Moves the required assets into the specified dir

        """

        # Copy assets
        for (model, size, wildcard) in self.asset_sizes:
            screen_dirs = self._get_model_image_dirs(source_dir, model, territory)

            for screen_dir in screen_dirs:
                asset_dir = os.path.join(screen_dir, size, wildcard)
                dst_asset = os.path.join(output_dir, size)

                copy_glob(asset_dir, dst_asset)

        # Copy .dat files
        ap_dat_file = self.get_ap_dat_path(source_dir, territory)
        op_dat_file = self.get_op_dat_path(source_dir)
        km_dat_file = self.get_keymgr_dat_path(source_dir)
        dst_dat_dir = os.path.join(output_dir, self.model)
        if not os.path.isdir(dst_dat_dir):
            os.makedirs(dst_dat_dir)
        shutil.copy(ap_dat_file, dst_dat_dir)
        # OP.DAT merged
        # shutil.copy(op_dat_file, dst_dat_dir)
        shutil.copy(op_dat_file, output_dir)
        shutil.copy(km_dat_file, dst_dat_dir)

        other_files = self.get_other_files()
        for (src_path, dst_path) in other_files:
            src = os.path.join(source_dir, src_path)
            dest = os.path.join(output_dir, dst_path)

            if not os.path.isdir(dest):
                os.makedirs(dest)
            shutil.copy(src, dest)


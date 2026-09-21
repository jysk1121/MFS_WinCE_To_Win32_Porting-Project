import os
from .base_model import BaseModel
from distutils.dir_util import copy_tree

SCREEN_SIZE = '1024_768'

class SE2800T(BaseModel):
    """
    A builder for the MX2800T

    """

    def __init__(self):
        super(SE2800T, self).__init__('2800', [
                ('2800', SCREEN_SIZE, '*'),
                ('2700', '720_480', 'OP_*'),
                ('2700', '1024_600', 'OP_*')
            ], '2800T')

    def get_other_files(self):
        """
        Returns a list of other misc. files, which aren't contemplated by the 
        standard build process.

        """
		# OP.DAT is merged into one now
        # return [
        #     (os.path.join("DAT", "Screen", "2700", "op.dat"), "2700"),
        # ]
        return []

    def get_op_dat_path(self, source_dir):
		# OP.DAT is merged into one now
        # model_dir = self._get_model_asset_dir(source_dir, '5200')
        model_dir = self._get_model_asset_dir(source_dir, "")
        return os.path.join(model_dir, "Op.dat")

    def get_keymgr_dat_path(self, source_dir):
        model_dir = self._get_model_asset_dir(source_dir, '5200')
        return os.path.join(model_dir, "KeyMgr.dat")

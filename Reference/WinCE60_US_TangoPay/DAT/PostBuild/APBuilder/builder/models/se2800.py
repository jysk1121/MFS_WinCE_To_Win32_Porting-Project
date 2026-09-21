import os
from .base_model import BaseModel
from distutils.dir_util import copy_tree

SCREEN_SIZE = '1024_768'

class SE2800(BaseModel):
    """
    A builder for the MX2800SE

    """

    def __init__(self):
        super(SE2800, self).__init__('2800', [
                ('2800', SCREEN_SIZE, '*'),
                ('2700', '1024_600', 'OP_*')
            ], '2800SE')

    def get_op_dat_path(self, source_dir):
		# OP.DAT is merged into one now
        # model_dir = self._get_model_asset_dir(source_dir, '5200')
        model_dir = self._get_model_asset_dir(source_dir, "")
        return os.path.join(model_dir, "Op.dat")
    
    def get_keymgr_dat_path(self, source_dir):
        model_dir = self._get_model_asset_dir(source_dir, '5200')
        return os.path.join(model_dir, "KeyMgr.dat")

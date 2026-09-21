import os
from .base_model import BaseModel
from distutils.dir_util import copy_tree

SCREEN_SIZE = '1024_600'

class SE2700(BaseModel):
    """
    A builder for the NH2700, NH2600, MX2600SE, and MX5000SE

    """

    def __init__(self):
        super(SE2700, self).__init__('2700', [
                ('2700', SCREEN_SIZE, '*')
            ], '2700SE')

import os
from .base_model import BaseModel
from distutils.dir_util import copy_tree

SCREEN_SIZE = '800_600'

class SE1800(BaseModel):
    """
    A builder for the 1800SE and MX4000W

    """

    def __init__(self):
        super(SE1800, self).__init__('1800SE', [
                ('1800SE', SCREEN_SIZE, '*')
            ], '1800SE')

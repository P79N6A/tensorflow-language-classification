
""" Simple memory database using memcache.
For whatever memcached server is running this, be sure to specify the -M option.

Example Usage::
    >>> from esp.utils.memdb import mem_db
    >>> mem_db.get('test')
    None
    >>> mem_db.set('test','hello')
    >>> mem_db.get('test')
    'hello'
"""

from django.core.cache import cache

__all__ = ['mem_db']

class MemDatabase(object):
    def __init__(self, server):
        self._cache = cache

    def get(self, key, default=None):
        val = self._cache.get(str(key))
        if val is None:
            return default
        else:
            return val

    def sync(self):
        self._cache.flush_all()

    def set(self, key, value, timeout=None):
        if isinstance(value, unicode):
            value = value.encode('utf-8')
        self._cache.set(str(key), value, timeout or 0)

    def delete(self, key):
        self._cache.delete(key)


mem_db = MemDatabase('localhost:11211')

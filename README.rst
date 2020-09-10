Сборка документации
===================

Подготовка среды разработчика
-----------------------------

Для подготовки среды выполнить::

  pip3.6 install pipenv --user
  export PATH=~/.local/bin:$PATH
  pipenv install

Сборка
------

Рекомендуется установить переменную окружения ``SPHINXFLAGS``, чтобы
предупреждения расценивались как ошибки::

  export SPHINXFLAGS=-W

Собрать HTML::

  pipenv run make -С doc html  # or make -C doc singlehtml

Для просмотра результата сборки открыть в браузере ``doc/build/html/index.html``.

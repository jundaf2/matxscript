��          D               l   �   m   m   :     �     �  }  �  �   H  �   �     �     �   Due to Global Interpreter Lock (GIL) implemented in Python interpreter, multithreading in Python can‘t be truly executed in parallel. In matx, we implement matx.pmap to support multithreading in native. Matx implements a builtin regular expression engine based on `PCRE <https://github.com/PCRE2Project/pcre2>`_. Multithreading Regular Expression Project-Id-Version: Matxscript 
Report-Msgid-Bugs-To: 
POT-Creation-Date: 2023-01-06 16:24+0800
PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE
Last-Translator: FULL NAME <EMAIL@ADDRESS>
Language: zh_CN
Language-Team: zh_CN <LL@li.org>
Plural-Forms: nplurals=1; plural=0;
MIME-Version: 1.0
Content-Type: text/plain; charset=utf-8
Content-Transfer-Encoding: 8bit
Generated-By: Babel 2.11.0
 原生 python 受限制于 全局解释器锁 (GIL)，多线程并不会真正并行执行，在 Matx 中，我们提供了 matx.pmap 原语，提供了后台并行执行能力。 Matx 内置了一个基于 `PCRE <https://github.com/PCRE2Project/pcre2>`_ 的高性能正则引擎，目前单独封装了接口，可以如下使用 并行支持 正则表达式 
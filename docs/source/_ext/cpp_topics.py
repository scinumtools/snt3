"""Render a header-based subset of a namespace with Breathe."""

from pathlib import Path
import xml.etree.ElementTree as ET

from breathe.directives.content_block import DoxygenNamespaceDirective
from breathe.renderer.filter import Filter, FilterFactory
from docutils.parsers.rst import directives
from sphinx.errors import ExtensionError


class SelectedSymbols(Filter):
    def __init__(self, identifiers):
        self.identifiers = identifiers

    def allow(self, node_stack):
        node = node_stack[0]
        return (getattr(node, 'id', None) or getattr(node, 'refid', None)) in self.identifiers


class TopicFilterFactory(FilterFactory):
    def __init__(self, app, identifiers):
        super().__init__(app)
        self.identifiers = identifiers

    def create_content_filter(self, kind, options):
        return super().create_content_filter(kind, options) & SelectedSymbols(self.identifiers)


class DoxygenTopic(DoxygenNamespaceDirective):
    option_spec = dict(DoxygenNamespaceDirective.option_spec, headers=directives.unchanged_required)

    @property
    def filter_factory(self):
        return TopicFilterFactory(self.env.app, self.identifiers)

    def run(self):
        repo = Path(self.env.srcdir).parents[1]
        headers = set()
        for pattern in self.options.get('headers', '').split():
            matches = list(repo.glob(pattern))
            if not matches:
                raise ExtensionError(f'C++ API header pattern matches no files: {pattern}')
            headers.update(path.resolve() for path in matches)
        if not headers:
            raise ExtensionError('doxygentopic requires :headers:')

        project = self.project_info_factory.create_project_info(self.options)
        xml = Path(project.project_path())
        index = xml / 'index.xml'
        self.env.note_dependency(str(index))
        namespace = self.arguments[0]
        self.identifiers = set()
        namespaces = []
        for entry in ET.parse(index).getroot().findall('compound'):
            name = entry.findtext('name', '')
            if name != namespace and not name.startswith(namespace + '::'):
                continue
            path = xml / (entry.get('refid') + '.xml')
            self.env.note_dependency(str(path))
            compound = ET.parse(path).find('compounddef')
            if entry.get('kind') == 'namespace':
                namespaces.append(name)
                candidates = compound.findall('./sectiondef/memberdef')
            else:
                candidates = [compound]
            for candidate in candidates:
                location = candidate.find('location')
                if location is not None and Path(location.get('file', '')).resolve() in headers:
                    self.identifiers.add(candidate.get('id'))

        if not self.identifiers:
            raise ExtensionError(f'No documented symbols found for {namespace} in the selected headers')

        # Content-only avoids repeatedly declaring the module namespace on
        # different topic pages. Breathe still qualifies each C++ declaration.
        self.options['content-only'] = None
        nodes = []
        for name in namespaces:
            self.arguments[0] = name
            nodes.extend(super().run())
        self.arguments[0] = namespace
        return nodes


def setup(app):
    app.add_directive('doxygentopic', DoxygenTopic)
    return {'version': '1', 'parallel_read_safe': True, 'parallel_write_safe': True}

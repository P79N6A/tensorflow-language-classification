/*
 * generated by Xtext 2.9.1
 */
package org.xtext.tortoiseshell.ui;

import com.google.inject.Injector;
import org.eclipse.xtext.ui.guice.AbstractGuiceAwareExecutableExtensionFactory;
import org.osgi.framework.Bundle;
import org.xtext.tortoiseshell.ui.internal.TortoiseshellActivator;

/**
 * This class was generated. Customizations should only happen in a newly
 * introduced subclass. 
 */
public class TortoiseShellExecutableExtensionFactory extends AbstractGuiceAwareExecutableExtensionFactory {

	@Override
	protected Bundle getBundle() {
		return TortoiseshellActivator.getInstance().getBundle();
	}
	
	@Override
	protected Injector getInjector() {
		return TortoiseshellActivator.getInstance().getInjector(TortoiseshellActivator.ORG_XTEXT_TORTOISESHELL_TORTOISESHELL);
	}
	
}
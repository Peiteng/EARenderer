//
//  MeshEditingTabViewItem.m
//  EARenderer
//
//  Created by Pavlo Muratov on 19.05.17.
//  Copyright © 2017 MPO. All rights reserved.
//

#import "SettingsTabViewItem.h"

#import "NSNumberFormatter+CustomFormatters.h"

@interface SettingsTabViewItem () <NSTextFieldDelegate>

@property (assign, nonatomic) EARenderer::RenderingSettings renderingSettings;

@property (weak, nonatomic) IBOutlet NSButton *renderMeshesEnabledButton;
@property (weak, nonatomic) IBOutlet NSButton *parallaxMappingEnabledButton;
@property (weak, nonatomic) IBOutlet NSTextField *parallaxMappingStrengthTextField;
@property (weak, nonatomic) IBOutlet NSButton *lightMultibounceEnabledButton;
@property (weak, nonatomic) IBOutlet NSButton *skyboxRenderingEnabledButton;
@property (weak, nonatomic) IBOutlet NSButton *triangleRenderingEnabledButton;
@property (weak, nonatomic) IBOutlet NSButton *frameMeasurementEnabledButton;

@property (weak, nonatomic) IBOutlet NSTextField *ESMFactorTextField;
@property (weak, nonatomic) IBOutlet NSTextField *ESMDarkeningTextField;
@property (weak, nonatomic) IBOutlet NSTextField *shadowBlurRadiusTextField;

@property (weak, nonatomic) IBOutlet NSButton *surfelRenderingEnabledButton;
@property (weak, nonatomic) IBOutlet NSPopUpButton *surfelRenderingModeButton;
@property (weak, nonatomic) IBOutlet NSTextField *surfelProbeIndexTextField;

@property (weak, nonatomic) IBOutlet NSButton *diffuseProbesRenderingEnabledButton;
@property (weak, nonatomic) IBOutlet NSTextField *probeSphereRadiusTextField;
@property (weak, nonatomic) IBOutlet NSTextField *linkedProbeIndexTextField;

@end

@implementation SettingsTabViewItem

#pragma mark - Lifecycle

- (void)awakeFromNib
{
    [super awakeFromNib];

    self.parallaxMappingStrengthTextField.floatValue = mSettings.meshSettings.parallaxMappingStrength;
    self.ESMFactorTextField.floatValue = mSettings.meshSettings.ESMFactor;
    self.ESMDarkeningTextField.floatValue = mSettings.meshSettings.ESMDarkeningFactor;
    self.shadowBlurRadiusTextField.intValue = mSettings.meshSettings.shadowBlurRadius;
    self.surfelProbeIndexTextField.intValue = mSettings.surfelSettings.POVProbeIndex;
}

#pragma mark - Actions

- (IBAction)meshRenderingToggled:(NSButton *)sender {
    mSettings.meshSettings.meshRenderingEnabled = sender.state == NSControlStateValueOn;
    [self.delegate settingsTabViewItem:self didChangeRenderingSettings:mSettings];
}

- (IBAction)parallaxMappingToggled:(NSButton *)sender {
    mSettings.meshSettings.parallaxMappingEnabled = sender.state == NSControlStateValueOn;
    [self.delegate settingsTabViewItem:self didChangeRenderingSettings:mSettings];
}

- (IBAction)parallaxMappingStrengthChanged:(NSTextField *)sender {
    mSettings.meshSettings.parallaxMappingStrength = sender.floatValue;
    [self.delegate settingsTabViewItem:self didChangeRenderingSettings:mSettings];
}

- (IBAction)applyMaterialsToggled:(NSButton *)sender {
    mSettings.meshSettings.materialsEnabled = sender.state == NSControlStateValueOn;
    [self.delegate settingsTabViewItem:self didChangeRenderingSettings:mSettings];
}

- (IBAction)globalIlluminationToggled:(NSButton *)sender {
    mSettings.meshSettings.globalIlluminationEnabled = sender.state == NSControlStateValueOn;
    [self.delegate settingsTabViewItem:self didChangeRenderingSettings:mSettings];
}

- (IBAction)lightMultibounceToggled:(NSButton *)sender {
    mSettings.meshSettings.lightMultibounceEnabled = sender.state == NSControlStateValueOn;
    [self.delegate settingsTabViewItem:self didChangeRenderingSettings:mSettings];
}

- (IBAction)skyboxRenderingToggled:(NSButton *)sender {
    mSettings.skyboxRenderingEnabled = sender.state == NSControlStateValueOn;
    [self.delegate settingsTabViewItem:self didChangeRenderingSettings:mSettings];
}

- (IBAction)triangleRenderingToggled:(NSButton *)sender {
    mSettings.triangleRenderingEnabled = sender.state == NSControlStateValueOn;
    [self.delegate settingsTabViewItem:self didChangeRenderingSettings:mSettings];
}

- (IBAction)frameMeasurementToggled:(NSButton *)sender {
//    mSettings. = sender.state == NSControlStateValueOn;
    [self.delegate settingsTabViewItem:self didChangeRenderingSettings:mSettings];
}

#pragma mark - Shadows

- (IBAction)ESMFactorChanged:(NSTextField *)sender {
    mSettings.meshSettings.ESMFactor = sender.floatValue;
    [self.delegate settingsTabViewItem:self didChangeRenderingSettings:mSettings];
}

- (IBAction)ESMDarkeningFactorChanged:(NSTextField *)sender {
    mSettings.meshSettings.ESMDarkeningFactor = sender.floatValue;
    [self.delegate settingsTabViewItem:self didChangeRenderingSettings:mSettings];
}

- (IBAction)shadowBlurRadiusChanged:(NSTextField *)sender {
    mSettings.meshSettings.shadowBlurRadius = sender.intValue;
    [self.delegate settingsTabViewItem:self didChangeRenderingSettings:mSettings];
}

#pragma mark - Surfels

- (IBAction)surfelRenderingToggled:(NSButton *)sender {
    mSettings.surfelSettings.renderingEnabled = sender.state == NSControlStateValueOn;
    [self.delegate settingsTabViewItem:self didChangeRenderingSettings:mSettings];
}

- (IBAction)surfelRenderingModeChanged:(NSPopUpButton *)sender {
    switch (sender.indexOfSelectedItem) {
        case 0: mSettings.surfelSettings.renderingMode = EARenderer::SurfelRenderer::Mode::Default; break;
        case 1: mSettings.surfelSettings.renderingMode = EARenderer::SurfelRenderer::Mode::Clusters; break;
    }
    [self.delegate settingsTabViewItem:self didChangeRenderingSettings:mSettings];
}

- (IBAction)surfelProbeIndexChanged:(NSTextField *)sender {

    [self.delegate settingsTabViewItem:self didChangeRenderingSettings:mSettings];
}

- (IBAction)diffuseProbeRenderingToggled:(NSButton *)sender {
    mSettings.probeSettings.probeRenderingEnabled = sender.state == NSControlStateValueOn;
    [self.delegate settingsTabViewItem:self didChangeRenderingSettings:mSettings];
}

- (IBAction)probeSphereRadiusChanged:(NSTextField *)sender {
    mSettings.probeSettings.sphereRadius = sender.floatValue;
    [self.delegate settingsTabViewItem:self didChangeRenderingSettings:mSettings];
}

- (IBAction)linkedProbeIndexChanged:(NSTextField *)sender {
    mSettings.probeSettings.clusterLinksRenderingProbeIndex = sender.intValue;
    [self.delegate settingsTabViewItem:self didChangeRenderingSettings:mSettings];
}


@end
import { IUnrealDescriptor } from "@pixstream/universal/bin/unreal";

export interface IReadyDescriptor extends IUnrealDescriptor {
    type: 'ready';
    message: string;
}

export type MyUnrealDescriptorTypes = IReadyDescriptor;